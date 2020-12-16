/**
 * File Name   : ExportSettingDetailViewController.swift
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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


protocol ExportSettingDelegate {
    func exportSettingValueChanged(_ picker:ExportSettingDetailViewController, value:ExportSetting)
}

class ExportSettingDetailViewController : UITableViewController {
    
    let bpsValues    = [ 512000,   1024000,  1536000,  2048000,  2560000,  3072000,  3584000,  4096000,
                        4608000,  5120000,  5632000,  6144000,  6656000,  7168000,  7680000,  8192000,
                        8704000,  9216000,  9728000,  10240000, 10752000, 11520000, 11618438, 12288000]
    
    @IBOutlet var width: UILabel!
    @IBOutlet var height: UILabel!
    
    @IBOutlet var fpsSlider: UISlider!
    @IBOutlet var fpsLabel: UILabel!
    @IBOutlet var bpsSlider: UISlider!
    @IBOutlet var bpsLabel: UILabel!

    var setting:ExportSetting!
    var delegate:ExportSettingDelegate!
    
    var bpsCurrentIndex: Int = 0 {
        willSet(index) {
            self.bpsSlider.value = Float(index)
            self.bpsLabel.text = StringTools.humanReadableBits(bpsValues[index])
        }
    }
    
    var bpsCurrentValue: Int {
        get {
            return bpsValues[bpsCurrentIndex]
        }
        set(value) {
            var found: Int? = nil
            if let index = bpsValues.index(of: value) {
                found = index
            } else {
                /// look for max(smaller than value)
                for (index, bps) in bpsValues.enumerated() {
                    if bps > value {
                        found = index
                        break
                    }
                }
                if found == nil {
                    found = bpsValues.count - 1
                }
            }
            bpsCurrentIndex = found!
        }
    }
    
    var fpsCurrent: Int = 1 {
        didSet(old) {
            let range = Int(self.fpsSlider.minimumValue)..<(Int(self.fpsSlider.maximumValue)+1)
            if !range.contains(fpsCurrent) {
                if fpsCurrent < range.lowerBound {
                    fpsCurrent = range.lowerBound
                } else {
                    fpsCurrent = range.upperBound
                }
            }
            self.fpsLabel.text = String(format: "%d fps", fpsCurrent)
            self.fpsSlider.value = Float(fpsCurrent)
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.bpsSlider.minimumValue = 0
        self.bpsSlider.maximumValue = Float(bpsValues.count - 1)
        
        self.initControls()
        self.navigationItem.rightBarButtonItem =
            UIBarButtonItem.init(title: "Reset", style: .plain, target: self, action:  #selector(didTapResetButton(sender:)))
    }
    
    @objc func didTapResetButton(sender:UIBarButtonItem) {
        self.initControls()
    }
    
    func initControls() {
        self.width.text = String(format: "%d", self.setting.width)
        self.height.text = String(format: "%d", self.setting.height)
        
        self.bpsCurrentValue = self.setting.bitrate
        self.fpsCurrent = self.setting.fps
    }
    
    @IBAction func fpsChanged(_ sender: UISlider) {
        let index = roundf(sender.value)
        sender.value = index
        self.fpsCurrent = Int(index)
    }
    
    @IBAction func bpsChanged(_ sender: UISlider) {
        let index = roundf(sender.value)
        sender.value = index
        self.bpsCurrentIndex = Int(index)
    }

    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        self.setting = ExportSetting(width: setting.width,
                                    height: setting.height,
                                    aspectType: setting.aspectType,
                                    bitrate: bpsValues[bpsCurrentIndex],
                                    fps: fpsCurrent)
        self.delegate.exportSettingValueChanged(self, value: self.setting)
    }
}
