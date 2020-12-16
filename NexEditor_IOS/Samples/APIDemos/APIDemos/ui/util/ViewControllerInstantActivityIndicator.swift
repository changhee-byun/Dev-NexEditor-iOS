/*
 * File Name   : ViewControllerInstantActivityIndicator.swift
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

import UIKit

extension UIViewController {

    func runWithActivityIndicator(style: UIActivityIndicatorView.Style = .whiteLarge, _ run: @escaping ()->Void ) {
        
        let activityIndicator = UIActivityIndicatorView(style: style)
        
        activityIndicator.center = CGPoint(x: self.view.bounds.midX, y: self.view.bounds.midY)
        self.view.addSubview(activityIndicator)
        
        activityIndicator.startAnimating()
        
        DispatchQueue.main.async {
            run()
            activityIndicator.stopAnimating()
            activityIndicator.removeFromSuperview()
        }
    }
    
    // Runs the closure in the main thread with activity indicator spinning at rightBarButtonItem
    func runWithBarButtonItemActivityIndicator(style: UIActivityIndicatorView.Style = .gray, _ run: @escaping ()->Void ) {
        let activityIndicator = UIActivityIndicatorView(style: style)

        let item = UIBarButtonItem(customView: activityIndicator)
        let savedRightBarButtonItems = self.navigationItem.rightBarButtonItems
        self.navigationItem.rightBarButtonItems = [item]
        
        activityIndicator.startAnimating()
        
        DispatchQueue.main.async {
            run()
            activityIndicator.stopAnimating()
            self.navigationItem.rightBarButtonItems = savedRightBarButtonItems
        }
    }
}
