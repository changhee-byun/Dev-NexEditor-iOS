/**
 * File Name   : SimpleEditorViewController.swift
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
import NexEditorFramework

enum AssetButtonType: Int {
    case templates = 0
    case titles
    
    static let images: [AssetButtonType : String] = [
        .templates: "icon-clipgraphics-press",
        .titles: "icon-text-press",
    ]
    static let categories: [AssetButtonType: String] = [
        .templates: NXEAssetItemCategory.template,
        .titles: NXEAssetItemCategory.textEffect,
    ]
    
    func image(for state: UIControlState) -> UIImage? {
        return UIImage(named: type(of:self).images[self] ?? "_notfound_")
    }
    
    var assetCategory: String {
        return type(of:self).categories[self]!
    }
}

enum PlaybackState: Int {
    case none
    case playing
    case paused
}

extension NXEProject {
    static func project(with clips: [NXEClip]) -> NXEProject {
        let project = NXEProject()
        project.visualClips = clips
        return project
    }
}

class SimpleEditorViewController: UIViewController {

    @IBOutlet var assetButtons: [UIButton]!
    @IBOutlet var buttonPlayPause: UIButton!
    @IBOutlet var editorPreview: NXEEngineView!
    @IBOutlet var collectionViewAssets: UICollectionView!
    @IBOutlet var collectionViewClips: UICollectionView!
    @IBOutlet var sliderPlayback: UISlider!
    
    enum SegueId: String {
        case transitionPicker = "transitionPicker"
        case effectPicker = "effectPicker"
    }
    
    let isMultiAssetPickerEnabled = true
    
    var editor: NXEEngine!
    
    var needsStartPlaying = false
    var selectedClipIndex: Int?
    
    var project: NXEProject! {
        willSet(newProject) {
            editor.setProject(newProject)
            editor.preparedEditor {
                self.buttonPlayPause.isEnabled = true
                if self.needsStartPlaying {
                    self.playPause(self.buttonPlayPause)
                    self.needsStartPlaying = false
                }
            }
        }
    }
    
    var clips: [NXEClip] = [] {
        willSet(newClips) {
            self.project = NXEProject.project(with: newClips)
        }
    }
    
    var clipSources: [ClipSource] = [] {
        willSet(newSources) {
            clipsDataSource = ClipSequenceDataSource(clips: newSources)
        }
    }
    
    var assetDataSource: AssetCollectionViewDataSource!
    var clipsDataSource: ClipSequenceDataSource! {
        didSet(oldValue) {
            clipsDataSource.on = { (event, clip, index) in
                self.selectedClipIndex = index
                if event == .didSelectTransition {
                    self.performSegue(withIdentifier: SegueId.transitionPicker.rawValue, sender: self)
                }
            }
            collectionViewClips.dataSource = clipsDataSource
            collectionViewClips.delegate = clipsDataSource
            collectionViewClips.reloadData()
        }
    }
    
    var playbackState: PlaybackState = .none {
        willSet(newState) {
            switch newState {
            case .none:
                buttonPlayPause.setImage(#imageLiteral(resourceName: "btn-preview-play-normal"), for: .normal)
                buttonPlayPause.setImage(#imageLiteral(resourceName: "btn-preview-play-press"), for: .highlighted)
                break
                
            case .playing:
                buttonPlayPause.setImage(#imageLiteral(resourceName: "btn-preview-pause-normal"), for: .normal)
                buttonPlayPause.setImage(#imageLiteral(resourceName: "btn-preview-pause-press"), for: .highlighted)
                break
                
            case .paused:
                buttonPlayPause.setImage(#imageLiteral(resourceName: "btn-preview-play-normal"), for: .normal)
                buttonPlayPause.setImage(#imageLiteral(resourceName: "btn-preview-play-press"), for: .highlighted)
                break
                
            }
        }
    }

    var selectedButtonType: AssetButtonType = .templates {
        willSet(newType) {
            for (index, button) in assetButtons.enumerated() {
                button.isSelected = (index == newType.rawValue)
            }
        }
    }
    
    
    // MARK: -
    override func viewDidLoad() {
        super.viewDidLoad()

        NXEEngine.setLogEnableErr(true, warning: true, flow: true)
        editor = NXEEngine.instance()
        editor.setPreviewWith(editorPreview.layer, previewSize: editorPreview.bounds.size)
        
        setupButtonImages()
        buttonPlayPause.isEnabled = false
        
        selectedButtonType = .templates
        DispatchQueue.global().async {
            let ds = AssetCollectionViewDataSource(category:AssetButtonType.templates.assetCategory)
            DispatchQueue.main.async {
                self.assetDataSource = ds
                self.collectionViewAssets.dataSource = ds
            }
        }
        collectionViewAssets.delegate = self
        
        clipsDataSource = ClipSequenceDataSource(clips: self.clipSources)
    }
    
    override func viewDidLayoutSubviews() {
        editor.setPreviewWith(editorPreview.layer, previewSize: editorPreview.bounds.size)
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    // MARK: -
    
    func setupButtonImages() {
        for (index, button) in assetButtons.enumerated() {
            let type = AssetButtonType(rawValue: index)!
            button.setImage(type.image(for:.selected), for: .selected)
        }
    }
    
    // TODO: Issue 1. Use of phasset-image:// prefix for image clip path is not explicitly forced or documented
    // TODO: Issue 2. Image/Video clip picking can't be quickly done using UIImagePickerController. Need a quick solution for App developers.

    func configureProject(with sources: [ClipSource]) {
        
        self.buttonPlayPause.isEnabled = false
        playbackState = .none
        
        var clips: [NXEClip] = []
        
        for source in sources {
            do {
                let clip = try NXEClip(source: source.nxeClipSource)
                clips.append(clip)
            } catch let error {
                print("\(error.localizedDescription)")
            }
        }
        // TODO: Issue 3. Keep AVURLAsset object retained while NXEClip needs the file system path to it.
        self.clipSources = sources
        self.clips = clips
    }
    
    // MARK: -
    var imagePicker: ImagePicker?
    
    @IBAction func open(_ sender: Any) {
        let type: ImagePicker.Type = isMultiAssetPickerEnabled ? DKImagePicker.self : SimpleImagePicker.self

        imagePicker = type.init(self) { picked, sources in
            self.configureProject(with: sources!)
            self.imagePicker = nil
        }
        imagePicker!.present()
        
    }

    @IBAction func switchAssetCollection(_ sender: UIButton) {
        let selected = assetButtons.index(of: sender)!
        selectedButtonType = AssetButtonType(rawValue: selected)!
        
        let type = selectedButtonType
        
        // switch collection
        DispatchQueue.global().async {
            let ds = AssetCollectionViewDataSource(category: type.assetCategory)
            DispatchQueue.main.async {
                self.assetDataSource = ds
                self.collectionViewAssets.dataSource = ds;
            }
        }
    }
    
    @IBAction func playPause(_ sender: UIButton) {
        
        switch playbackState {
        case .none:
            editor.play()
            // TODO: Issue 4. progress update comes much, 1.5 sec, earlier than start
            editor.setCallbackWithPlayProgress({ (currentTime) in
                self.sliderPlayback.value = Float(currentTime)
            }, playStart: {
                self.sliderPlayback.minimumValue = 0
                self.sliderPlayback.maximumValue = Float(self.project.getTotalTime())
            }, playEnd: {
                self.playbackState = .none
            })
            playbackState = .playing
            break
            
        case .playing:
            editor.pause()
            playbackState = .paused
            break
            
        case .paused:
            editor.resume()
            playbackState = .playing
            break
        }
    }
    
    override func shouldPerformSegue(withIdentifier identifier: String, sender: Any?) -> Bool {
        var result: Bool = false
        if let segid = SegueId(rawValue:identifier) {
            if segid == .transitionPicker || segid == .effectPicker {
                result = selectedClipIndex != nil
            }
        }
        return result
        
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) -> Void {
        if self.playbackState == .playing {
            playPause(self.buttonPlayPause)
        }
        
        if segue.identifier == SegueId.effectPicker.rawValue || segue.identifier == SegueId.transitionPicker.rawValue {
            if let segueId = SegueId(rawValue: segue.identifier!) {
                let nav = segue.destination as! UINavigationController
                let dest = nav.viewControllers[0] as! EffectPickerViewController
                prepareEffectPicker(segueId: segueId, destination: dest)
            }
        }
    }
    
    func prepareEffectPicker(segueId: SegueId, destination: EffectPickerViewController) {
        let clip = clips[selectedClipIndex!]
        let isPickingEffect = segueId == .effectPicker
        
        destination.effectCategory = isPickingEffect ? NXEAssetItemCategory.effect : NXEAssetItemCategory.transition
        
        let savedSpot = self.editor.getCurrentPosition()
        destination.on = { [unowned self] (event, viewController, effectItem) in
            let savedClips = self.clips
            self.clips = []
            if let effectItem = effectItem, event == .didSelect {
                if isPickingEffect {
                    clip.clipEffectID = effectItem.itemId
                } else {
                    clip.transitionEffectID = effectItem.itemId
                    clip.setTransitionEffectDuration(1000)
                }
            }
            viewController.dismiss(animated: true) {
                self.refreshClips(savedClips)
                self.editor.seek(savedSpot)
            }
        }
    }
    
    // MARK: -
    func refreshClips(_ clips: [NXEClip]) {
        self.clips = clips
    }

}

extension SimpleEditorViewController: UICollectionViewDelegate {
    func collectionView(_ collectionView: UICollectionView, shouldSelectItemAt indexPath: IndexPath) -> Bool {
        var result = false
        if let cell = collectionView.cellForItem(at: indexPath) {
            result = self.clips.count > 0 && !cell.isSelected
        }
        return result
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        if let cell = collectionView.cellForItem(at: indexPath) {
            cell.backgroundColor = UIColor.blue.withAlphaComponent(0.7)
        }
        if let item = self.assetDataSource .item(at: indexPath) {
            if selectedButtonType == .templates, self.clips.count > 0 {
                if self.playbackState == .playing {
                    editor.pause()
                    self.playbackState = .paused
                }
                
                let project = try? NXETemplateProject(templateAssetItem: item as! NXETemplateAssetItem, clips: self.clips)
                self.project = project
                self.needsStartPlaying = true
            }
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, didDeselectItemAt indexPath: IndexPath) {
        if let cell = collectionView.cellForItem(at: indexPath) {
            cell.backgroundColor = UIColor.darkGray
        }
    }
}

