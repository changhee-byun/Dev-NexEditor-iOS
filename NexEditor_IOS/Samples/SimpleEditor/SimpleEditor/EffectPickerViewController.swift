/**
 * File Name   : EffectPicker.swift
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
import NexEditorFramework

class NexEditorView: UIView {
    
    var projectHolder: EditorProjectHolder!
    var editor: NXEEngine {
        return projectHolder.editor
    }

    public override init(frame: CGRect) {
        super.init(frame: frame)
        commonInit()
    }
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        commonInit()
    }
    
    private func commonInit() -> Void {
        projectHolder = EditorProjectHolder(stateChanged: { [unowned self] clipsReady in
            if clipsReady {
                self.editor.play()
            }
        })
        
        projectHolder.editor.setPreviewWith(self.layer, previewSize: self.bounds.size)
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
        projectHolder.editor.setPreviewWith(self.layer, previewSize: self.bounds.size)
    }
    
    func play() -> Void {
        editor.play()
    }
    
    func stop() -> Void {
        editor.stopASync { (errorcode) in
            if errorcode != .ERROR_NONE {
                print(errorcode)
            }
        }
    }
}

extension String: ClipSource {
    var nxeClipSource: NXEClipSource {
        return NXEClipSource(path: self)
    }
    
    func loadThumbnail(size: CGSize, _ loaded: @escaping (_ image: UIImage?)->Void )
    {
        loaded(UIImage(contentsOfFile: self)?.resized(size))
    }
}

class EffectPickerViewController: UIViewController {
    enum Event {
        case didSelect
        case didCancel
    }

    // MARK: -
    @IBOutlet var preview: NexEditorView!
    @IBOutlet var collectionView: UICollectionView!
    @IBOutlet var buttonDone: UIBarButtonItem!
    
    // MARK: -
    public var on: (Event, EffectPickerViewController, NXEAssetItem?) -> Void = { (_, _, _) in }
    public var effectCategory: String = NXEAssetItemCategory.transition
    
    fileprivate var selectedIndexPath: IndexPath? {
        didSet(old) {
            buttonDone.isEnabled = selectedIndexPath != nil
        }
    }
    
    fileprivate var dataSource: AssetCollectionViewDataSource! {
        didSet(old) {
            collectionView.dataSource = dataSource
        }
    }
    
    // MARK: -
    @IBAction func done(_ sender: Any) {
        if let item = dataSource.item(at: selectedIndexPath!) {
            self.preview.projectHolder.editor.stopASync { (error) in
                if error != .ERROR_NONE {
                    print("Stop error: \(error.rawValue)")
                }
                self.on(.didSelect, self, item)
            }
        }
    }
    
    @IBAction func cancel(_ sender: Any) {
        self.preview.projectHolder.editor.stopASync { (error) in
            if error != .ERROR_NONE {
                print("Stop error: \(error.rawValue)")
            }
            self.on(.didCancel, self, nil)
        }
    }
    
    // MARK: -
    override func viewDidLoad() {
        super.viewDidLoad()
        buttonDone.isEnabled = selectedIndexPath != nil

        collectionView.delegate = self
        DispatchQueue.global().async {
            let dataSource = AssetCollectionViewDataSource(category: self.effectCategory)
            DispatchQueue.main.async {
                self.dataSource = dataSource
                self.collectionView.reloadData()
            }
        }
        
        // red to blue
        let clips = [NXEClip.newSolidClip("#FFFF0000")!, NXEClip.newSolidClip("#FF0000FF")!]
        for clip in clips {
            clip.setImageClipDuration(1500)
        }
        preview.projectHolder.clips = clips
        
        title = "Select \(effectCategory.capitalized)"
        
    }
}

extension EffectPickerViewController: UICollectionViewDelegate {
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        guard let cell = collectionView.cellForItem(at: indexPath) else {
            return
        }
        cell.backgroundColor = UIColor.blue.withAlphaComponent(0.7)
        selectedIndexPath = indexPath
        
        if let item = self.dataSource.item(at: indexPath) {
            let clips = self.preview.projectHolder.clips
            
            if effectCategory == NXEAssetItemCategory.transition {
                clips[0].transitionEffectID = item.itemId
                clips[0].setTransitionEffectDuration(1000)
            } else if effectCategory == NXEAssetItemCategory.effect {
                clips[0].clipEffectID = item.itemId
            }
            
            self.preview.projectHolder.clips = clips
            let title = NXETextLayer(text: item.itemId, font: nil, point: CGPointNXELayerCenter)!
            self.preview.projectHolder.editor.project.layers = [title]
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, didDeselectItemAt indexPath: IndexPath) {
        if let cell = collectionView.cellForItem(at: indexPath) {
            cell.backgroundColor = UIColor.darkGray
            selectedIndexPath = nil
        }
    }
}
