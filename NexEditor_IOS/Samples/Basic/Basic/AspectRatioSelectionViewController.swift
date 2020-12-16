/**
 * File Name   : AspectRatioSelectionViewController.swift
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

class AspectRatioSelectionViewController: UITableViewController {
    
    private var ratios: [AspectRatio] = [
        AspectRatio(h:16, v:9),
        AspectRatio(h:9, v:16),
        AspectRatio(h:1, v:1),
        AspectRatio(h:4, v:3),
        AspectRatio(h:3, v:4),
        AspectRatio(h:3, v:1),
        AspectRatio(h:1, v:3),
        AspectRatio(h:640, v:480),
    ]

    private var selectedIndex: Int = 0
    
    var done: ((_ ratio: AspectRatio) -> Void)? = nil
    
    var ratio: AspectRatio = AspectRatio(h:16, v:9) {
        willSet(newValue) {
            var index: Int?
            index = ratios.index(of: newValue)
            if index == nil {
                ratios.append(newValue)
                index = ratios.count - 1
            }
            self.selectedIndex = index!
        }
    }
    
    // MARK: -
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return ratios.count
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "cellAspectRatio", for: indexPath)
        let ratio = ratios[indexPath.row]
        cell.textLabel!.text = ratio.displayText
        
        if indexPath.row == selectedIndex {
            cell.accessoryType = .checkmark
        } else {
            cell.accessoryType = .none
        }
        return cell
    }
    
    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        selectedIndex = indexPath.row
        done?(ratios[selectedIndex])
    }

}
