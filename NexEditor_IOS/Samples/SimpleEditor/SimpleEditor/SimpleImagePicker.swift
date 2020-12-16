/**
 * File Name   : SimpleImagePicker
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

import UIKit
import MobileCoreServices
import Photos

protocol ImagePicker {
    typealias ImagePicked = (_ picked: Bool, _ sources: [ClipSource]?)->Void
    init(_ viewController: UIViewController, didPick:@escaping ImagePicked )
    func present()
}

class SimpleImagePicker: ImagePicker {

    let delegate: UIImagePickerControllerDelegateSimple
    let viewController: UIViewController
    let picker: UIImagePickerController
    
    required init(_ viewController: UIViewController, didPick:@escaping ImagePicker.ImagePicked ) {
        self.viewController = viewController
        
        delegate = UIImagePickerControllerDelegateSimple() { picked, sources in
            viewController.dismiss(animated: true, completion: { 
                didPick(picked, sources)
            })
        }
        picker = UIImagePickerController()
        picker.delegate = delegate
        picker.sourceType = .photoLibrary
        picker.mediaTypes = [kUTTypeImage as String, kUTTypeMovie as String]
    }
    
    func present() {
        viewController.present(picker, animated: true)
    }
}

class UIImagePickerControllerDelegateSimple: NSObject, UIImagePickerControllerDelegate, UINavigationControllerDelegate {
    
    let block: ImagePicker.ImagePicked
    
    init(_ block: @escaping ImagePicker.ImagePicked) {
        self.block = block
        super.init()
    }
    
    static func clipSource(with url: URL, mediaType: String) -> ClipSource? {
        var result: ClipSource? = nil
        
        if mediaType == kUTTypeImage as String {
            if let asset = PHAsset.fetchAssets(withALAssetURLs: [url], options: nil).firstObject {
                result = asset
            }
            
        } else if mediaType == kUTTypeMovie as String {
            result = AVURLAsset(url: url)

        }
        return result
    }
    
    func imagePickerController(_ picker: UIImagePickerController, didFinishPickingMediaWithInfo info: [String : Any]) {
        
        var url: URL?
        let type = info[UIImagePickerControllerMediaType] as? String
        if let type = type {
            if type == kUTTypeImage as String {
                url = info[UIImagePickerControllerReferenceURL] as? URL
            } else if type == kUTTypeMovie as String {
                url = info[UIImagePickerControllerMediaURL] as? URL
            }
        }
        
        var source: ClipSource? = nil
        if let url = url, let type = type {
            source = type(of:self).clipSource(with:url, mediaType: type)
        }
        
        self.block(source != nil, [source!])
    }
    
    func imagePickerControllerDidCancel(_ picker: UIImagePickerController) {
        self.block(false, nil)
    }
}
