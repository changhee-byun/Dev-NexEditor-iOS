//
//  NexEditorPreviewViewController.swift
//  QuickPrototypeVideoPlayer
//
//  Created by Simon Kim on 11/22/17.
//  Copyright © 2017 NexStreaming Crop. All rights reserved.
//

import UIKit
import NexEditorFramework
import DKImagePickerController

class NexEditorPreviewViewController: UIViewController {
    @IBOutlet var previewArea: UIView!
    @IBOutlet var buttonPlayPause: UIButton!
    @IBOutlet var preview: UIView!
    @IBOutlet var seekSlider: UISeekSlider!

    private var editor: NXEEngine!
    
    var playable: Playable!
    var buttonTarget: ButtonTarget!

    deinit {
        seekSlider.onSeek = nil
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()

        editor = NXEEngine.instance()
        editor.setPreviewWith(preview.layer, previewSize: preview.bounds.size)
                
        playable = NexEditorPlayable(editor: editor)
        
        playable.addObserver(seekSlider)
        playable.addObserver(AdhocPlayableStateObserver({
            [weak buttonPlayPause] (playable, state, status) in
            guard let button = buttonPlayPause else { return }
            if state == .playing {
                let imageName = status.playing ? "icon-pause" : "icon-play"
                button.setImage(UIImage(named: imageName), for: .normal)
            }
            if state == .loaded {
                button.isEnabled = status.loaded
            }
        }))
        seekSlider.onSeek = { [weak self] (time) in
            self?.playable.seek(to: time)
        }
        seekSlider.value = 0

        buttonTarget = ButtonTarget(button: buttonPlayPause)
        buttonTarget.onTap = {
            [weak self] (sender) in
            guard let me = self else { return }
            guard me.playable.status.loaded else { return }
            
            if me.playable.status.playing {
                me.playable.pause()
            } else {
                if me.playable.duration > kCMTimeZero && me.playable.currentTime == me.playable.duration {
                    me.playable.seek(to: kCMTimeZero)
                }
                me.playable.play()
            }
        }
        
        editor.preparedEditor { [weak self] in
            guard let me = self else { return }
            if let project = me.loadSampleClip() {
                me.editor.setProject(project)
                me.editor.seek(0)
                me.playable.changeState(.loaded, to: true)
            }
        }

    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        editor.setPreviewWith(preview.layer, previewSize: preview.bounds.size)
    }

    func loadSampleClip() -> NXEProject? {
        var result: NXEProject? = nil
        if let url = Bundle.main.url(forResource: "sample1", withExtension: "mp4", subdirectory: "bundle") {
            let asset = AVURLAsset(url: url)
            let clipSource = NXEClipSource(avurlAsset: asset)
            do {
                let clip = try NXEClip(source: clipSource)
                
                let project = NXEProject()
                project.visualClips = [clip]
                result = project
            } catch let e {
                print(e)
            }
        }
        return result
    }
    
    func setProjectWith(assets: [PHAsset]) -> NXEProject? {
        var result: NXEProject? = nil
        let clipSources = assets.map({ NXEClipSource(phAsset:$0) })
        if let clips = try? clipSources.map( { try NXEClip(source: $0)}) {
            let project = NXEProject()
            project.visualClips = clips
            result = project

            self.playable.changeState(.loaded, to: false)
            self.editor.setProject(project)
            self.editor.seek(0)
            self.playable.changeState(.loaded, to: true)
        }
        return result
    }
    
    @IBAction func newClips(_ sender: Any) {
        let picker = DKImagePickerController()
        picker.didSelectAssets = { [weak self] (assets) in
            let phassets = assets.map( {$0.originalAsset!})
            _ = self?.setProjectWith(assets: phassets)
        }
        present(picker, animated: true)
    }
}
