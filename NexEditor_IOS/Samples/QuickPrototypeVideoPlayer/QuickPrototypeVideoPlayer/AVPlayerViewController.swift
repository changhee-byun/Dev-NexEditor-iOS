//
//  ViewController.swift
//  QuickPrototypeVideoPlayer
//
//  Created by Simon Kim on 11/22/17.
//  Copyright © 2017 NexStreaming Crop. All rights reserved.
//

import UIKit
import AVFoundation

class AVPlayerViewController: UIViewController {
    @IBOutlet var previewArea: UIView!
    @IBOutlet var preview: UIView!
    @IBOutlet var buttonPlayPause: UIButton!
    @IBOutlet var seekSlider: UISeekSlider!
    
    var player = AVPlayer()
    var playerLayer: AVPlayerLayer!
    var playable: Playable!
    var buttonTarget: ButtonTarget!
    
    deinit {
        seekSlider.onSeek = nil
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        playerLayer = AVPlayerLayer(player: player)
        preview.layer.insertSublayer(playerLayer, at: 0)
                
        playable = AVPlayerPlayable(with: player)
        
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
        
        if let url = Bundle.main.url(forResource: "sample1", withExtension: "mp4", subdirectory: "bundle") {
            let playerItem = AVPlayerItem(url: url)
            player.replaceCurrentItem(with: playerItem)
        }        
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        playerLayer.frame = self.preview.bounds
    }
}

