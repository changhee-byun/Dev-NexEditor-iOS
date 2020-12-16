/**
 * File Name   : ViewControllerSimpleAlert.swift
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

enum SimpleAlertType {
    case error
    case warning
    case information
    case custom(String)
    
    var displayText: String {
        let result: String
        switch self {
        case .custom(let title): result = title; break
        case .error: result = "Error"; break
        case .warning: result = "Warning"; break
        case .information: result = "Information"; break
        }
        return result
    }
}

/// Simply call alert(.information, "Please select a clip"), in a UIViewController instance method
/// And UIAlertController will be presented something like below,
///
///   --------------------
///       Information
///
///   Please select a clip
///         [ OK ]
///   --------------------
extension UIViewController {
    func alert(_ type: SimpleAlertType, _ message: String, buttonTitle: String = "OK") {
        alert(type.displayText, message: message, buttonTitle: buttonTitle)
    }
    
    @objc open func alert(_ title: String, message: String, buttonTitle: String = "OK") {
        alert(message, title: title, buttonTitle: buttonTitle)
    }
    
    /// For Objective-C access simplicity
    @objc open func alert(_ message: String, title: String) {
        alert(message, title: title, buttonTitle: "OK")
    }
    
    @objc open func alert(_ message: String, title: String, buttonTitle: String = "OK", complete: @escaping (()->Void) = {} ) {
        let alert = UIAlertController(title: title, message: message, preferredStyle: .alert)
        
        let ok = UIAlertAction(title: buttonTitle, style: .default) { (action) in
            complete()
            alert.dismiss(animated: true, completion: nil)
        }
        alert.addAction(ok)
        self.present(alert, animated: true, completion: nil)
    }
    
    @objc open func alertSimpleActionSheet(_ title: String, message: String, actionTitles: [String], dismissTitle: String, complete: @escaping (_ actionIndex: Int)->Void ) {
        let alert = UIAlertController(title: title, message: message, preferredStyle: .actionSheet)
        
        let actionHandler: (UIAlertAction) -> Void = { action in
            let index = actionTitles.index(of: action.title!) ?? NSNotFound
            complete(index)
        }
        
        for title in actionTitles {
            alert.addAction(UIAlertAction(title: title, style: .default, handler: actionHandler))
        }
        alert.addAction(UIAlertAction(title: dismissTitle, style: .cancel) { action in
            complete(NSNotFound)
        })
        
        present(alert, animated: true)
    }
}
