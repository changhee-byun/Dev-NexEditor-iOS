/**
 * File Name   : MediaPickerViewController.swift
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

class MediaPickerOptionKeys: NSObject {
    @objc static var video = "video"
    @objc static var image = "image"
    @objc static var audio = "audio"
    @objc static var singleSelect = "singleSelect"
}

struct MediaPickerOptions {
    var video = true
    var image = true
    var audio = false
    var singleSelect = false
    let showInfoButton: Bool
    
    init(video: Bool, image: Bool, audio: Bool, singleSelect: Bool, showInfoButton: Bool = false) {
        self.video = video
        self.image = image
        self.audio = audio
        self.singleSelect = singleSelect
        self.showInfoButton = showInfoButton
    }
    
    init() {
        self.init(video: true, image: true, audio: false, singleSelect: false)
    }
    
    /// For Objective-C compatibility
    init(dictionary: [String: Bool]) {
        self.init(video: dictionary[MediaPickerOptionKeys.video] ?? false,
                  image: dictionary[MediaPickerOptionKeys.image] ?? false,
                  audio: dictionary[MediaPickerOptionKeys.audio] ?? false,
                  singleSelect: dictionary[MediaPickerOptionKeys.singleSelect] ?? false)
    }
    
    var dictionary: [String: Bool] {
        var result: [String: Bool] = [:]
        result[MediaPickerOptionKeys.video] = video
        result[MediaPickerOptionKeys.image] = image
        result[MediaPickerOptionKeys.audio] = audio
        result[MediaPickerOptionKeys.singleSelect] = singleSelect
        return result
    }
}

extension IndexPath {
    var uiViewTag: Int {
        return (self.row << 16) | self.section
    }
    
    init(withUIViewTag tag: Int) {
        let row = tag >> 16
        let section = tag & 0xFFFF
        self.init(row: row, section: section)
    }
}

/// Implement this protocol and provide as delegate property of MediaPickerViewController
protocol MediaPickerDelegate : ItemPickerDelegate {
    func mediaPicker(_ picker: MediaPickerViewController, didSelect rows: [IndexPath])
    func mediaPicker(_ picker: MediaPickerViewController, didTapInfoButtonAtIndexPath indexPath: IndexPath)
}

@objc protocol MediaPickerObjcDelegate: ItemPickerObjcDelegate {
    func mediaPicker(_ picker: MediaPickerViewController, didSelectRows rows: [IndexPath])
    @objc optional func mediaPicker(_ picker: MediaPickerViewController, didTapInfoButtonAtIndexPath indexPath: IndexPath)
}

/// Convert MediaPickerObjcDelegate to MediaPickerDelegate
class MediaPickerDelegateConverter: ItemPickerDelegateConverter, MediaPickerDelegate {
    
    func mediaPicker(_ picker: MediaPickerViewController, didSelect rows: [IndexPath])
    {
        guard let objcDelegate = objcDelegate as? MediaPickerObjcDelegate else { return }
        objcDelegate.mediaPicker(picker, didSelectRows: rows)
    }
    
    func mediaPicker(_ picker: MediaPickerViewController, didTapInfoButtonAtIndexPath indexPath: IndexPath)
    {
        guard let objcDelegate = objcDelegate as? MediaPickerObjcDelegate else { return }
        objcDelegate.mediaPicker?(picker, didTapInfoButtonAtIndexPath: indexPath)
    }
}

protocol MediaPickerDataSource: ItemPickerDataSource {
    func shouldShowInfoButton(at indexPath: IndexPath) -> Bool
}

class MediaPickerViewController: ItemPickerCollectionViewController {
    
    
    var options = MediaPickerOptions()
    

    
    /// For Objective-C access
    @objc var optionsDictionary: [String: Bool] {
        set(value) {
            self.options = MediaPickerOptions(dictionary: value)
        }
        
        get {
            return self.options.dictionary
        }
    }
    
    override func delegateConverter(_ objcDelegate: ItemPickerObjcDelegate) -> ItemPickerDelegateConverter {
        return MediaPickerDelegateConverter(objcDelegate)
    }
        
    override func awakeFromNib() {
        super.awakeFromNib()
    }
    
    // MARK: -
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.title = "Select Media"
        collectionView!.allowsMultipleSelection = !options.singleSelect
    }
    
    override func didMove(toParent parent: UIViewController?) {
        if parent == nil {
            self.delegate?.itemPickerDidCancel(self)
        }
    }
    
    func selectItems(with titles:[String]) {
        ///
        for title in titles {
            selectItem(with: title)
        }
    }
    
    func selectItem(with title:String) {
        let sections = self.dataSource?.sectionCount
        for i in 0...(sections!-1) {
            let itemCount = self.dataSource?.itemCount(in: i)
            for j in 0...(itemCount!-1) {
                let indexPath = IndexPath(row: j, section: i)
                if let item = self.dataSource?.item(at: indexPath) {
                    if item.title == title {
                        collectionView!.selectItem(at: indexPath, animated: false, scrollPosition: [])
                        rerender(at: indexPath)
                        selections.append(indexPath)
                        return
                    }
                }
            }
        }
    }
    
    func deselectAll() {
        if let indexPaths = collectionView!.indexPathsForSelectedItems {
            for indexPath in indexPaths {
                collectionView!.deselectItem(at: indexPath, animated: false)
                rerender(at: indexPath)
                selections = selections.filter({ $0 != indexPath} )
            }
        }
    }
    
    @objc func buttonTapped(_ sender:UIButton) {
        if let delegate = self.delegate as? MediaPickerDelegate {
            let indexPath = IndexPath(withUIViewTag: sender.tag)
            delegate.mediaPicker(self, didTapInfoButtonAtIndexPath: indexPath)
        }
    }
    
    // MARK: -
    var selections: [IndexPath] = []
    override func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        rerender(at: indexPath)
        selections.append(indexPath)
    }
    
    override func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let cell = super.collectionView(collectionView, cellForItemAt: indexPath)
        
        if let button = cell.contentView.firstSubview(ofClass:UIButton.self) {
            let showButton = (self.dataSource as? MediaPickerDataSource)?.shouldShowInfoButton(at: indexPath) ?? false
            button.isHidden = !showButton
            
            if showButton {
                button.tag = indexPath.uiViewTag
                button.addTarget(self, action:#selector(buttonTapped(_:)), for: .touchUpInside)
            }
        }
        return cell
    }
    
    override func collectionView(_ collectionView: UICollectionView, didDeselectItemAt indexPath: IndexPath) {
        rerender(at: indexPath)
        selections = selections.filter({ $0 != indexPath} )
    }

    // MARK: -
    @IBAction func nextClick(_ sender: Any) {
        
        if selections.count > 0 {
            if let delegate = self.delegate as? MediaPickerDelegate {
                delegate.mediaPicker(self, didSelect: selections)
            } else {
                self.alert(.error, "MediaPickerDelegate was not set")
            }
        } else {
            self.alert(.information, "Please select item to proceed")
        }
    }
}

/// Alternative to MediaPickerDelegate wheren blocks are favored
class MediaPickerDelegateBlock: NSObject, MediaPickerDelegate {

    enum Event {
        case cancel
        case didSelect // [IndexPath]
        case didTapInfo
    }
    
    public typealias MediaPickerBlock = (_ picker: MediaPickerViewController, _ event: Event, _ rows: [IndexPath]) -> (Void)

    let block: MediaPickerBlock
    
    init(_ block: @escaping MediaPickerBlock ) {
        self.block = block
        super.init()
    }
    
    func mediaPicker(_ picker: MediaPickerViewController, didSelect rows: [IndexPath]) {
        block(picker, .didSelect, rows)
    }
    
    func mediaPicker(_ picker: MediaPickerViewController, didTapInfoButtonAtIndexPath indexPath: IndexPath) {
        block(picker, .didTapInfo, [indexPath])
    }
    
    func itemPicker(_ picker: ItemPickerController, didSelect indexPath: IndexPath) {
        block(picker as! MediaPickerViewController, .didSelect, [indexPath])
    }
    
    func itemPickerDidCancel(_ picker: ItemPickerController) {
        block(picker as! MediaPickerViewController, .cancel, [])
    }
}

class MediaPickerPresenter {
    let onCancel: (()->Void)
    let options: MediaPickerOptions
    
    private var delegateBlock: MediaPickerDelegate!
    
    @objc func mediaPickerDidCancel(_ sender: Any?) {
        // viewController.dismiss(animated: true) {}
        onCancel()
    }
    
    func present(from viewController: UIViewController, _ didSelect: @escaping ((_ selection: MediaPickerSelection)->Void)) {
        let storyboard = UIStoryboard(name: "AssetPickers", bundle: nil)
        let nav = storyboard.instantiateViewController(withIdentifier: "modalMediaPicker") as! UINavigationController
        let destination = nav.viewControllers[0] as! MediaPickerViewController
        destination.options = options
        destination.dataSource = MediaPickerDataSourceImpl(options: options)
        
        delegateBlock = MediaPickerDelegateBlock() { [weak viewController] (picker, event, rows) in
            
            var afterDismiss: (()->Void)? = nil
            if event == .didSelect {
                afterDismiss = {
                    let selection = (picker.dataSource as! MediaPickerDataSourceImpl).selection(with: rows)
                    didSelect(selection)
                }
            }
            viewController?.dismiss(animated: true, completion: afterDismiss)
        }
        destination.delegate = delegateBlock
        destination.navigationItem.leftBarButtonItem = UIBarButtonItem(barButtonSystemItem: .cancel, target: self, action: #selector(mediaPickerDidCancel(_:)))
        
        viewController.present(nav, animated: true) {}
    }
    
    init(_ options: MediaPickerOptions = MediaPickerOptions(video: true, image: true, audio: false, singleSelect: false),
         _ onCancel: @escaping (()->Void)) {
        self.options = options
        self.onCancel = onCancel
    }
}
