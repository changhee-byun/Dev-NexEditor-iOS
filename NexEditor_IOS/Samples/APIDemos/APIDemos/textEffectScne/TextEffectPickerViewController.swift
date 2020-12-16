/**
 * File Name   : TextEffectPickerViewController.swift
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

/// Objective
@objc protocol TextEffectPickerDelegate: ItemPickerObjcDelegate {
    func itemPicker(_ picker: TextEffectPickerViewController,
                    didSelect indexPath: IndexPath,
                    itemId: String,
                    params: NXETextEffectParams)
}

class TextEffectPickerDelegateConverter: ItemPickerDelegateConverter {
    override func itemPicker(_ picker: ItemPickerController, didSelect indexPath: IndexPath)
    {
        if picker is TextEffectPickerViewController,
            objcDelegate is TextEffectPickerDelegate {
            
            let picker = picker as! TextEffectPickerViewController
            let objcDelegate = self.objcDelegate as! TextEffectPickerDelegate
            let item = picker.dataSource?.item(at: indexPath) as! AssetPickerItem
            objcDelegate.itemPicker(picker,
                                    didSelect: indexPath,
                                    itemId: item.assetItem.itemId,
                                    params: picker.textEffectParams)
        } else {
            self.objcDelegate.itemPicker(picker as! ItemPickerCollectionViewController, didSelect: indexPath)
        }
    }
    
}

/// Set objcDelegate and textEffectParams properties before presenting and get the resulting text effect asset item id and text effect params in the delegate method
class TextEffectPickerViewController: ItemPickerCollectionViewController {
    /// From Objective-C, set this property instead of delegate property
    @objc public var textEffectParams: NXETextEffectParams = NXETextEffectParams()

    /// Create delegate converter that will translate delegate methods to objc
    override func delegateConverter(_ objcDelegate: ItemPickerObjcDelegate) -> ItemPickerDelegateConverter {
        return TextEffectPickerDelegateConverter(objcDelegate)
    }
    
    // MARK: -

    override func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        // Discard cell selection and trigger the segue instead.
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier != "segueTextEffectTitle" {
            return
        }
        guard let indexPaths = collectionView!.indexPathsForSelectedItems else { return }
        let indexPath = indexPaths[0]
        
        let nav = segue.destination as! UINavigationController
        let vc = nav.viewControllers[0] as! TextEffectTitleEntriesViewController
        
        vc.texts = textEffectParams.titles
        vc.onComplete = {[weak delegate] viewController, entered, texts in
            self.dismiss(animated: true, completion: { 
                if let texts = texts, entered == true {
                    self.textEffectParams.titles = texts
                    delegate?.itemPicker(self, didSelect: indexPath)
                }
            })
        }
    }
}

/// For Objective-C caller to set text effect picker data source which is available only to Swift
extension TextEffectPickerViewController {
    @objc func setDefaultDataSource(with aspectType: NXEAspectType) {
        dataSource = TextEffectPickerDataSource(aspectType)
    }
}

extension NXETextEffectParams {
    var titles: [String] {
        get {
            return [
                self.introTitle ?? "",
                self.introSubtitle ?? "",
                self.outroTitle ?? "",
                self.outroSubtitle ?? ""
            ]
        }
        
        set {
            var newTitles: [String] = newValue
            while(newTitles.count < 4) {
                newTitles.append("")
            }
            self.introTitle = newTitles[0]
            self.introSubtitle = newTitles[1]
            self.outroTitle = newTitles[2]
            self.outroSubtitle = newTitles[3]
        }
    }
}
