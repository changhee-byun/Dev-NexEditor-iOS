//
//  ColorAdjustmentView.swift
//  APIDemos
//
//  Created by jake.you on 9/7/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import Foundation
import UIKit

@objc protocol ColorAdjustmentDelegate {
    func colorAdjustmentView(_ view: ColorAdjustmentView, didChangeValue value: NXEColorAdjustments )
    func colorAdjustmentView(_ view: ColorAdjustmentView, willDisappearWithValue value: NXEColorAdjustments )
}

class ColorAdjustmentView: UIView {
    
    @objc var delegate: ColorAdjustmentDelegate? = nil
    
    @objc var colorAdjustments:NXEColorAdjustments = NXEColorAdjustmentsMake(0,0,0) {
        didSet {
            updateUI()
        }
    }
    
    private let labels = [ UILabel(), UILabel(), UILabel()]
    private let sliders = [UISlider(), UISlider(), UISlider()]
    private let texts = ["Brightness", "Contrast", "Saturation"]
    
    override init(frame: CGRect) {
        super.init(frame: frame)
        setup()
    }
    
    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override func willMove(toSuperview newSuperview: UIView?) {
        if let delegate = self.delegate {
            delegate.colorAdjustmentView(self, willDisappearWithValue:NXEColorAdjustmentsMake(self.sliders[0].value, self.sliders[1].value, self.sliders[2].value))
        }
    }
    
    private func updateUI() {
        sliders[0].value = colorAdjustments.brightness
        labels[0].text = texts[0] + " (\(String(format: "%.2f", colorAdjustments.brightness)))"
            
        sliders[1].value = colorAdjustments.contrast
        labels[1].text = texts[1] + " (\(String(format: "%.2f", colorAdjustments.contrast)))"
            
        sliders[2].value = colorAdjustments.saturation
        labels[2].text = texts[2] + " (\(String(format: "%.2f", colorAdjustments.saturation)))"
    }
    
    private func setupLayout(for view:UIView, _ topAnchor: NSLayoutYAxisAnchor, _ topAnchorConstant:CGFloat, _ height:CGFloat) -> Void {
        
        let margins = self.layoutMarginsGuide
        
        view.translatesAutoresizingMaskIntoConstraints = false
        view.leadingAnchor.constraint(equalTo: margins.leadingAnchor, constant: 0).isActive = true
        view.trailingAnchor.constraint(equalTo: margins.trailingAnchor, constant: 0).isActive = true
        view.topAnchor.constraint(equalTo: topAnchor, constant: topAnchorConstant).isActive = true
        NSLayoutConstraint(item: view, attribute: .height, relatedBy: .equal, toItem: nil, attribute: .notAnAttribute, multiplier: 1.0, constant: height).isActive = true
    }
    
    @objc func sliderValueDidChange(_ sender:UISlider!)
    {
        if let delegate = self.delegate {
            if let index = self.sliders.index(of: sender) {
                
                self.labels[index].text = texts[index] + " (\(String(format: "%.2f", sender.value)))"
                var newColorAdjustments = colorAdjustments
                
                if index == 0 { // Brightness
                    newColorAdjustments.brightness = sender.value
                }
                else if index == 1 { // Contrast
                    newColorAdjustments.contrast = sender.value
                }
                else if index == 2 { // Saturation
                    newColorAdjustments.saturation = sender.value
                }
                delegate.colorAdjustmentView(self, didChangeValue: newColorAdjustments)
                self.colorAdjustments = newColorAdjustments
            }
        }
    }
    
    @objc func reset (_ sender:UIButton!)
    {
        let value = Float(0);
        for (index, text) in texts.enumerated() {
            self.labels[index].text = text + " (\(String(format: "%.2f", value)))"
            self.sliders[index].setValue(value, animated: false)
        }
        colorAdjustments = NXEColorAdjustmentsMake(0, 0, 0)
        
        if let delegate = self.delegate {
            delegate.colorAdjustmentView(self, didChangeValue: colorAdjustments )
        }
    }
    
    private func setup() {
        
        self.backgroundColor = UIColor.white
        let margins = self.layoutMarginsGuide
        
        let labelTitle = UILabel()
        self.addSubview(labelTitle)
        
        labelTitle.text = "Color Adjustment"
        let font =  UIFont.boldSystemFont(ofSize: 20)
        
        labelTitle.font = font
        setupLayout(for: labelTitle, margins.topAnchor, 10, 20)
        labelTitle.textAlignment = .center
        
        for (index, label) in labels.enumerated() {
            label.text = texts[index]
            self.addSubview(label)
            
            let slider = sliders[index]
            slider.minimumValue = -1
            slider.maximumValue = 1
            slider.value = 0
            slider.addTarget(self, action: #selector(ColorAdjustmentView.sliderValueDidChange(_:)), for: .valueChanged)
            
            self.addSubview(slider)
            
            // layout
            if index == 0 {
                setupLayout(for:label, labelTitle.bottomAnchor, 20, 20)
            }
            else {
                setupLayout(for:label, sliders[index-1].bottomAnchor, 10, 20)
            }
            setupLayout(for:slider, label.bottomAnchor, 10, 20)
        }
        
        let resetBtn = UIButton()
        self.addSubview(resetBtn)
        resetBtn.backgroundColor = UIColor.red
        resetBtn.setTitle("Reset", for: UIControl.State.normal)
        resetBtn.addTarget(self, action: #selector(ColorAdjustmentView.reset(_:)), for: .touchUpInside)
        setupLayout(for:resetBtn, sliders[sliders.count-1].bottomAnchor, 20, 25)
    }
}
