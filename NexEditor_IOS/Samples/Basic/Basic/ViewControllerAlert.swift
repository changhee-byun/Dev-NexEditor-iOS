/**
 * File Name   : ViewControllerAlert.swift
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

extension UIViewController {

    func alertActions(_ title: String, message: String, actionTitles: [String], dismissTitle: String, complete: @escaping (_ actionIndex: Int?)->Void ) {
        let alert = UIAlertController(title: title, message: message, preferredStyle: .actionSheet)
        
        let actionHandler: (UIAlertAction) -> Void = { action in
            let index = actionTitles.index(of: action.title!)
            complete(index)
        }
        
        for title in actionTitles {
            alert.addAction(UIAlertAction(title: title, style: .default, handler: actionHandler))
        }
        alert.addAction(UIAlertAction(title: dismissTitle, style: .cancel) { action in
            complete(nil)
        })
        
        present(alert, animated: true)
    }
    
    func alertSimple(_ title: String, message: String, dismissTitle: String, dismissed: @escaping () -> Void) -> UIAlertController {
        let alert = UIAlertController(title: title, message: message, preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: dismissTitle, style: .default) { action in
            dismissed()
        })
        return alert
    }
    
    func alert(_ title: String, message: String) {
        let alert = alertSimple(title, message: message, dismissTitle: "OK") {
            
        }
        present(alert, animated: true)
    }
    
    
    /**
     * Alert message with a simple OK button.
     *
     * - Parameters:
     *   - message: message to be displaed
     *   - timeInterval: optional duration until dismiss. Default is nil.
     */
    func presentAlert(_ message: String,
                      title: String = "Information",
                      dismissButtonTitle: String? = "OK",
                      vanishIn timeInterval: TimeInterval? = nil) -> UIAlertController {
        
        
        let alert = UIAlertController(title: title, message: message, preferredStyle: .alert)
        if let dismissButtonTitle = dismissButtonTitle {
            let action = UIAlertAction(title: dismissButtonTitle, style: .default) { (UIAlertAction) in }
            alert.addAction(action)
        }
        
        let presentBlock = {
            self.present(alert, animated: true) {
                if let timeInterval = timeInterval {
                    self.dismissIf(presented: alert, in: timeInterval)
                }
            }
        }
        
        if presentedViewController is UIAlertController {
            // dismiss UIAlertController if already presented from somewhere else
            dismiss(animated: false) {
                presentBlock()
            }
        } else {
            presentBlock()
        }
        
        return alert
    }
    
    /**
     * @note Don't let textEntered(_:) capture objects to be freed later. UIAlertController has a leak issue as of iOS 9 since iOS 8
     */
    func alertTextInput( _ message: String, placeholder: String = "", defaultText: String = "", textEntered:@escaping (_: String?)->Void) {
        let alert = UIAlertController(title: "Information", message: message, preferredStyle: .alert)
        alert.addTextField(configurationHandler: { (textField) in
            textField.placeholder = placeholder
            textField.text = defaultText
        })
        
        let ok = UIAlertAction(title: "OK", style: .default, handler: { (aletAction) in
            textEntered(alert.textFields![0].text)
        })
        alert.addAction(ok)
        self.present(alert, animated: true, completion: {
            
        })
        
    }
    
    /// Alert message with "OK" button, "Information" title
    func alert(_ message: String) {
        _ = presentAlert(message, dismissButtonTitle: "OK")
    }
    
    /// Alert message for 0.5 seconds without title
    func shortAlert(_ message: String) {
        _ = presentAlert(message, title: "", dismissButtonTitle: nil, vanishIn: 0.5)
    }
    
    /**
     * Dismiss viewController if presented until timeInterval
     *
     * - Parameters:
     *   - viewController: view controller will be dismissed if yet presented
     *   - timeInterval: Duration to wait until dismiss
     */
    private func dismissIf(presented viewController: UIViewController, in timeInterval: TimeInterval)
    {
        _ = BlockTimer(withTimeInterval: timeInterval, repeats: false, block: { (blockTimer) in
            if self.presentedViewController == viewController {
                self.dismiss(animated: true, completion: nil)
            }
        })
    }    
}
