/**
 * File Name   : DKImagePicker.swift
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
import Photos
import DKImagePickerController

class DKImagePicker: ImagePicker {
    let picker: DKImagePickerController
    let viewController: UIViewController
    
    required init(_ viewController: UIViewController, didPick: @escaping ImagePicker.ImagePicked) {
        self.viewController = viewController
        picker = DKImagePickerController()
        picker.sourceType = .both
        picker.didSelectAssets = { items in
            var sources: [ClipSource] = []
            for item in items {
                sources.append(item.originalAsset!)
            }
            didPick(true, sources)
        }
    }
    
    func present() {
        viewController.present(picker, animated: true) { }
        
    }
}
