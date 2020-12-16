/**
 * File Name   : MediaPickerSceneTransitionComplication.swift
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

class PreviewMenuItemSegueHandler: NSObject, MenuItemSegueHandler {
    let menuItem: MenuItem
    let segueId: String
    let followingSegueId: String
    let mediaPickerOptions: MediaPickerOptions
    
    var mediaPickerSelection: MediaPickerSelection?

    func prepareSegue(_ segue: UIStoryboardSegue, sender: Any?) {
        guard segue.identifier == segueId  else { return }
        
        let destination = segue.destination as! MediaPickerViewController
        destination.options = mediaPickerOptions
        destination.delegate = self
        destination.dataSource = MediaPickerDataSourceImpl(options: mediaPickerOptions)
        destination.extraActions = [
            ExtraAction(title: "Test Preset VVII", action: { picker in
                guard let picker = picker as? MediaPickerViewController else { return }
                picker.deselectAll()
                picker.selectItems(with: ["tvc30sec.mp4", "tvc3sec.mp4", "Koala.jpg", "Tulips.jpg"])
            }),
            ExtraAction(title: "Test Preset IVIV", action: { picker in
                guard let picker = picker as? MediaPickerViewController else { return }
                picker.deselectAll()
                picker.selectItems(with: ["Tulips.jpg", "tvc30sec.mp4", "Koala.jpg", "tvc3sec.mp4"])
            }),
        ]
    }
    
    func displayMediaInformation(of item: MediaPickerItemData, from viewController: UIViewController) {
        viewController.alert(item.desc ?? "Unknown", title: "Media Information")
    }

    // MARK: -
    init(_ menuItem: MenuItem, options: MediaPickerOptions, segueId: String, followingSegueId: String)
    {
        self.menuItem = menuItem
        self.segueId = segueId
        self.mediaPickerOptions = options
        self.followingSegueId = followingSegueId
        super.init()
    }
}

extension PreviewMenuItemSegueHandler: MediaPickerDelegate, ItemPickerSegueDelegate {
    // MARK: - MediaPickerDelegate
    func itemPicker(_ picker: ItemPickerController, didSelect indexPath: IndexPath) {
        
    }
    
    func mediaPicker(_ picker: MediaPickerViewController, didSelect rows: [IndexPath]) {
        // Keep selected media items for later use
        if let dataSource = picker.dataSource as? MediaPickerDataSourceImpl {
            let selection = dataSource.selection(with: rows)
            self.mediaPickerSelection = selection
            picker.performSegue(withIdentifier: self.followingSegueId, sender: self)
        }
    }
    
    func itemPickerDidCancel(_ picker: ItemPickerController) {
        self.mediaPickerSelection = nil
    }
    
    // MARK: - ItemPickerSegueDelegate
    /// Invoked by Media Picker when a subsequent segue should be performed, according to Storyboard design.
    /// Currently, the segue is to present Preview view controller
    func itemPicker(_ picker: ItemPickerController, prepareFor segue: UIStoryboardSegue, sender: Any?) {
        guard let selection = mediaPickerSelection  else { return }

        if let segueId = segue.identifier {
            if segueId == "beattemplate" {
                let destination = segue.destination as! BeatTemplateViewController
                destination.clipSources = selection.clipSources
            }
            else {
                let destination = segue.destination as! PreviewViewController
                destination.setClipSources(selection.clipSources);
            }
        }
    }
    
    func mediaPicker(_ picker: MediaPickerViewController, didTapInfoButtonAtIndexPath indexPath: IndexPath) {
        if let dataSource = picker.dataSource as? MediaPickerDataSourceImpl {
            let item = dataSource.item(at:indexPath) as! MediaPickerItemData
            displayMediaInformation(of: item, from: picker)
        }
    }
}
