/*
 * File Name   : SlotEditView.swift
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
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

class SlotEditView: UIView {
    private let radmax = Double.pi * 2
    private let maxscale = 2.0
    
    public var onTap: (_ view: SlotEditView )->Void = { (v) in }
    public var onScale: (_ view: SlotEditView, _ scale: Double )->Void = { (v, s) in }
    public var onRotate: (_ view: SlotEditView, _ angle: Double )->Void = { (v, a) in }
    public var onMove: (_ view: SlotEditView, _ began: Bool, _ move: CGPoint )->Void = { (v, b, m) in }
    public var angle: Double = 0.0      // radian
    {
        didSet(old) {
            if angle < 0.0 {
                angle = 0.0
            } else if angle > radmax {
                angle = radmax
            }
        }
    }
    public var scale: Double = 1.0      // 1.0 ~
    {
        didSet(old) {
            if scale < 1.0 {
                scale = 1.0
            } else if scale > maxscale {
                scale = maxscale
            }
        }
    }

    public var focusRect = CGRect() {
        didSet(old) {
            if shapeFocus == nil {
                let shape = CAShapeLayer()
                shape.strokeColor = UIColor.white.cgColor
                shape.lineWidth = 2
                shape.lineJoin = CAShapeLayerLineJoin.bevel
                shape.fillColor = UIColor.clear.cgColor
                self.layer.addSublayer(shape)
                shapeFocus = shape
            }
            guard let shapeFocus = shapeFocus else { return }
            
            let bezierPath = UIBezierPath(rect: focusRect)
            shapeFocus.path = bezierPath.cgPath
        }
    }
    
    private var shapeFocus: CAShapeLayer?
    private var panDirection: PanDirection = .none
    private var panTranslation = CGPoint()
    
    
    private func commonInit() {
        self.isOpaque = false
        let tap = UITapGestureRecognizer(target: self, action: #selector(didTap(_:)))
        addGestureRecognizer(tap)
        
        let pan = UIPanGestureRecognizer(target: self, action: #selector(didPan(_:)))
        addGestureRecognizer(pan)
    }
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        commonInit()
    }
    
    override init(frame: CGRect) {
        super.init(frame: frame)
        commonInit()
    }
    
    override func draw(_ rect: CGRect) {
        guard let context = UIGraphicsGetCurrentContext() else { return }
        let fillColor = UIColor.black.withAlphaComponent(0.3).cgColor
        context.setFillColor(fillColor)
        context.addRect(focusRect)
        context.addRect(.infinite)
        context.fillPath(using: .evenOdd)
    }
    
    @objc private func didTap(_ sender: UITapGestureRecognizer) {
        onTap(self)
    }

    enum PanDirection {
        case none
        case horizontal
        case vertical
    }

    var isPanningWithinFocusRect = false
    
    @objc private func didPan(_ sender: UIPanGestureRecognizer) {
        switch(sender.state) {
        case .began:
            panDirection = .none
            panTranslation = CGPoint()
            isPanningWithinFocusRect = focusRect.contains(sender.location(in: self))
            if isPanningWithinFocusRect {
                onMove(self, true, CGPoint())
            }
            break
        case .changed, .ended:
            
            let translation = sender.translation(in: self)
            if isPanningWithinFocusRect {
                onMove(self, false, translation)
                break
            }
            
            if panDirection == .none {
                let velocity = sender.velocity(in: self)
                if abs(velocity.x) > abs(velocity.y) {
                    panDirection = .horizontal
                } else {
                    panDirection = .vertical
                }
            }

            if panDirection == .horizontal {
                panTranslation.x = translation.x
                // angle

                let angled = Double(panTranslation.x / self.bounds.size.width) * radmax
                let angle = min( max(0.0, self.angle + angled), radmax)
                if sender.state == .ended {
                    self.angle = angle
                }
                onRotate(self, angle)
            } else {
                panTranslation.y = translation.y

                let scaled = Double( -panTranslation.y / self.bounds.size.height)
                let scale = min(max(1.0, self.scale + scaled), maxscale)
                if sender.state == .ended {
                    self.scale = scale
                }
                onScale(self, scale)
            }
            break
        default:
            break
        }
    }
}
