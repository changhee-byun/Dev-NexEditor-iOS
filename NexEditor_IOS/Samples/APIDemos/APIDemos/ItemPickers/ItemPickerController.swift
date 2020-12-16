/**
 * File Name   : ItemPickerController.swift
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

protocol ItemPickerSegueDelegate {
    func itemPicker(_ picker: ItemPickerController, prepareFor segue: UIStoryboardSegue, sender: Any?)
}

protocol ItemPickerDelegate: NSObjectProtocol {
    func itemPicker(_ picker: ItemPickerController, didSelect indexPath: IndexPath)
    func itemPickerDidCancel(_ picker: ItemPickerController)
}

protocol ItemPickerController {
    var delegate: ItemPickerDelegate? { get set }
    var dataSource: ItemPickerDataSource? { get set }
    var itemView: ItemPickerItemView { get set }
}

struct BarButtonItemStatus {
    var barButton: UIBarButtonItem
    var enabled: Bool
}

protocol ItemPickerUIViewController: class, ItemPickerController  {
    var isLoading: Bool { get set }
    var navigationItem: UINavigationItem { get }
    var view: UIView! { get }
    var savedBarButtonsStatus: [BarButtonItemStatus] { get set }
}

extension ItemPickerUIViewController {
    
    func setLoading(status: Bool) {
        if status {
            savedBarButtonsStatus = startLoading()
        } else {
            stopLoading(status: savedBarButtonsStatus)
        }
    }
    
    func startLoading() -> [BarButtonItemStatus] {
        let activity = UIActivityIndicatorView(style: .gray)
        var buttonItems = navigationItem.rightBarButtonItems ?? []
        let result: [BarButtonItemStatus] = buttonItems.map({BarButtonItemStatus(barButton: $0, enabled: $0.isEnabled)} )
        
        for button in buttonItems {
            button.isEnabled = false
        }
        buttonItems.append(UIBarButtonItem(customView: activity))
        navigationItem.rightBarButtonItems = buttonItems
        
        activity.startAnimating()
        view.isUserInteractionEnabled = false
        return result
    }
    
    func stopLoading(status: [BarButtonItemStatus]) {
        self.view.isUserInteractionEnabled = true
        
        self.navigationItem.rightBarButtonItems = status.map( {
            $0.barButton.isEnabled = $0.enabled
            return $0.barButton
        } )
    }
    
}

