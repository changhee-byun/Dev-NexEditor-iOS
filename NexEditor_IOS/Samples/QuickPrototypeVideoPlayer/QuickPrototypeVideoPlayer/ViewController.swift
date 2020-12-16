//
//  ViewController.swift
//  QuickPrototypeVideoPlayer
//
//  Created by Simon Kim on 11/22/17.
//  Copyright © 2017 NexStreaming Crop. All rights reserved.
//

import UIKit
import AVFoundation

class ButtonTarget {
    var onTap: ((UIButton)->Void)? = nil
    let button: UIButton
    
    init(button: UIButton) {
        self.button = button
        button.addTarget(self, action: #selector(didTap(_:)), for: .touchUpInside)
    }
    @objc func didTap(_ sender: UIButton) {
        onTap?(sender)
    }
}

class AVPlayerViewController: UIViewController {
    @IBOutlet var preview: UIView!
    @IBOutlet var buttonPlayPause: UIButton!
    @IBOutlet var seekSlider: SeekSlider!
    
    var player = AVPlayer()
    var playerLayer: AVPlayerLayer!
    var playable: Playable!
    var buttonTarget: ButtonTarget!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        playerLayer = AVPlayerLayer(player: player)
        preview.layer.addSublayer(playerLayer)
        
        // workaround to put the button on top of the preview region
        buttonPlayPause.removeFromSuperview()
        preview.addSubview(buttonPlayPause)
        
        if let url = Bundle.main.url(forResource: "sample1", withExtension: "mp4", subdirectory: "bundle") {
            let playerItem = AVPlayerItem(url: url)
            player.replaceCurrentItem(with: playerItem)
        }
        
        playable = AVPlayerPlayable(with: player)
        seekSlider.seekTarget = playable
        
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
        
        buttonTarget = ButtonTarget(button: buttonPlayPause)
        buttonTarget.onTap = {
            [weak self] (sender) in
            guard let me = self else { return }
            guard me.playable.status.loaded else { return }
            
            if me.playable.status.playing {
                me.player.pause()
            } else {
                if me.playable.duration > kCMTimeZero && me.playable.currentTime == me.playable.duration {
                    me.player.seek(to: kCMTimeZero)
                }
                me.player.play()
            }
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

