//
//  BeatTemplateViewController.swift
//  APIDemos
//
//  Created by jake.you on 08/02/2018.
//  Copyright © 2018 NexStreaming Corp. All rights reserved.
//

import Foundation

class BeatTemplateViewController: UIViewController, UICollectionViewDelegate {
    
    private let cellReuseId = "cell"
    
    @IBOutlet weak var preview: NXELayerEditorView!
    @IBOutlet weak var playPauseButton: UIButton!
    @IBOutlet weak var currentTime: UILabel!
    @IBOutlet weak var contentDuration: UILabel!
    @IBOutlet weak var slider: UISeekSlider!
    
    @IBOutlet weak var collectionView: UICollectionView!
    
    fileprivate var editor: NXEEngine!
    fileprivate var project: NXEProject!
    fileprivate var playable: Playable!
    
    fileprivate var asset:NXEBeatAssetItem?
    fileprivate var assetGroups:[NXEAssetItemGroup]!
    fileprivate var hideControllers:Bool = false
    fileprivate var collectionViewDataSource:UICollectionViewDataSource?
    
    var clipSources:[NXEClipSource]?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let tap = UITapGestureRecognizer(target: self, action: #selector(didTap(_:)))
        self.preview.addGestureRecognizer(tap)
        
        editor = NXEEngine.instance()
        self.assetGroups = NXEAssetLibrary.instance().groups(inCategory: NXEAssetItemCategory.beatTemplate)

        self.collectionView.isUserInteractionEnabled = true
        self.collectionView.tintAdjustmentMode = .normal
        self.collectionView.tintColor = UIColor.gray
        self.collectionView.delegate = self

        //if let group = self.assetGroups.first, let asset = group.items[0] as? NXEBeatAssetItem {
        if let group = self.assetGroups.first, let asset = group.items[0] as? NXEBeatAssetItem {
            self.asset = asset
            self.collectionViewDataSource = ThumbnailSequenceDataSource(items: self.assetGroups, cellReuseId: self.cellReuseId)
            self.collectionView.dataSource = self.collectionViewDataSource
            self.collectionView.performBatchUpdates({
            }) { (finished) in
                let indexPath = IndexPath(row: 0, section: 0)
                self.collectionView.selectItem(at: indexPath, animated: true, scrollPosition: UICollectionView.ScrollPosition.left)
                if let cell = self.collectionView.cellForItem(at: indexPath) {
                    cell.setBorderColor(UIColor.blue)
                }
            }
            
            playable = NexEditorPlayable(editor: editor)
            playable.addObserver(slider)
            playable.addObserver(AdhocPlayableStateObserver({
                [weak self] (playable, state, status) in
                if state == .playing {
                    let imageNameDefault = status.playing ? "pause_default" : "play_default"
                    self?.playPauseButton.setImage(UIImage(named:imageNameDefault), for: .normal)
                }
                if state == .loaded {
                    self?.playPauseButton.isEnabled = status.loaded
                    self?.contentDuration.text = StringTools.formatTime(interval: TimeInterval(playable.duration.seconds))
                    let time = status.loaded ? 0 : playable.currentTime.seconds
                    self?.currentTime.text = StringTools.formatTime(interval: time)
                }
                if state == .currentTime {
                    self?.currentTime.text = StringTools.formatTime(interval: TimeInterval(playable.currentTime.seconds))
                }
            }))
            slider.onSeek = { [weak self] (time) in
                self?.playable.seek(to: time)
            }
            self.preparePlayback(editor: editor, asset: asset)
        }
        else {
            self.playPauseButton.isEnabled = false
            self.alert(.error, "No BeatTemplate found\n Install templates under\n nexeditor/pacakges")
        }
    }
    
    func preparePlayback(editor:NXEEngine, asset:NXEBeatAssetItem) -> Void {
        var clips = [NXEClip]()
        if let clipSources = self.clipSources {
            for clipSource in clipSources {
                do {
                    let clip = try NXEClip(source: clipSource)
                    clips.append(clip)
                }
                catch let e {
                    print(e.localizedDescription)
                }
            }
            do {
                project = try NXEBeatTemplateProject(beatTemplateAssetItem: asset, clips:clips)
                editor.setProject(project)
                editor.setPreviewWith(preview.layer, previewSize: preview.bounds.size)
                playable.seek(to: CMTime.zero)
                playable.changeState(.loaded, to: true)
            }
            catch let e {
                print(e.localizedDescription)
            }
        }
    }
    
    @IBAction func actionPlayPause(_ sender: UIButton) {
        if self.asset != nil {
            let playing = playable.status.playing
            if playing {
                playable.pause()
            } else {
                if playable.isStoppedAtAlmostEnd {
                    playable.seek(to: CMTime.zero)
                }
                playable.play()
            }
        }
    }
    override func viewDidLayoutSubviews() {
        DispatchQueue.main.async {
            self.editor.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
            self.preview.bringSubviewToFront(self.slider)
            self.preview.bringSubviewToFront(self.playPauseButton)
            self.preview.bringSubviewToFront(self.contentDuration)
            self.preview.bringSubviewToFront(self.currentTime)
        }
    }
    
    @objc func didTap(_ sender: UITapGestureRecognizer) {
        DispatchQueue.main.async {
            self.hideControllers = !self.hideControllers
            self.slider.isHidden = self.hideControllers
            self.playPauseButton.isHidden = self.hideControllers
            self.contentDuration.isHidden = self.hideControllers
            self.currentTime.isHidden = self.hideControllers
            if let collectionViewHeight = self.collectionView.constraints.filter( {$0.identifier == "height"}).first {
                if self.hideControllers {
                    collectionViewHeight.constant = 0
                } else {
                    collectionViewHeight.constant = 90
                }
            }
            self.navigationController?.setNavigationBarHidden(self.hideControllers, animated: true)
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        if let asset = self.assetGroups[indexPath.item].items.first as? NXEBeatAssetItem {
            self.asset = asset
            self.preparePlayback(editor: self.editor, asset: asset)
            for cell in self.collectionView.visibleCells {
                cell.setBorderColor(nil)
            }
            if let cell = self.collectionView.cellForItem(at: indexPath) {
                cell.setBorderColor(UIColor.blue)
            }
            playable.play()
        }
    }
}

