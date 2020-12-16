/**
 * File Name   : SettingsViewController.swift
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
import UIKit

class SettingsViewController: UITableViewController {
    @IBOutlet var labelAspectRatio: UILabel!
    
    struct Settings {
        var aspectRatio: AspectRatio = AspectRatio()
    }
    
    var settings: Settings = Settings()
    
    var done: ((_ settings: Settings) -> Void)? = nil
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.labelAspectRatio.text = self.settings.aspectRatio.displayText
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "aspectratio" {
            let destination = segue.destination as! AspectRatioSelectionViewController
            destination.ratio = settings.aspectRatio
            destination.done = { ratio in
                self.settings.aspectRatio = ratio
                _ = self.navigationController?.popViewController(animated: true)
                
                self.labelAspectRatio.text = ratio.displayText
            }
        }
    }
    
    override func didMove(toParentViewController parent: UIViewController?) {
        if parent == nil {
            done?(settings)
        }
    }
}
