//: Playground - noun: a place where people can play

import UIKit
import AVFoundation

import PlaygroundSupport

let view = UIView(frame: CGRect(x: 0, y: 0, width: 300, height: 200))
view.backgroundColor = UIColor.red
PlaygroundPage.current.liveView = view

let marginsGuide = view.layoutMarginsGuide

let slider = SeekSlider(frame: CGRect(x: 0, y: 0, width: 10, height: 32))
view.addSubview(slider)
slider.backgroundColor = UIColor.orange
slider.translatesAutoresizingMaskIntoConstraints = false
slider.leadingAnchor.constraint(equalTo: marginsGuide.leadingAnchor, constant: 0).isActive = true
slider.trailingAnchor.constraint(equalTo: marginsGuide.trailingAnchor, constant: 0).isActive = true
NSLayoutConstraint(item: slider,
                   attribute: .height,
                   relatedBy: .equal,
                   toItem: nil,
                   attribute: .notAnAttribute,
                   multiplier: 1.0,
                   constant: 32).isActive = true

let playerView = UIView(frame: CGRect(x: 0, y: 0, width: 222, height: 160))
view.addSubview(playerView)
playerView.backgroundColor = UIColor.green

playerView.translatesAutoresizingMaskIntoConstraints = false
playerView.leadingAnchor.constraint(equalTo: marginsGuide.leadingAnchor, constant: 0).isActive = true
playerView.trailingAnchor.constraint(equalTo: marginsGuide.trailingAnchor, constant: 0).isActive = true
playerView.topAnchor.constraint(equalTo: marginsGuide.topAnchor, constant: 0).isActive = true
playerView.bottomAnchor.constraint(equalTo: slider.topAnchor, constant: 4).isActive = true
NSLayoutConstraint(item: playerView,
                   attribute: .height,
                   relatedBy: .equal,
                   toItem: nil,
                   attribute: .notAnAttribute,
                   multiplier: 1.0,
                   constant: 148).isActive = true

let button = UIButton(frame: CGRect(x:0, y:0, width: 32, height: 32))
view.addSubview(button)
button.setImage(UIImage(named:"icon-play"), for: .normal)

let url = Bundle.main.url(forResource: "sample1", withExtension: "mp4")!
let playerItem = AVPlayerItem(url: url)

//: #. AVPlayer
var player = AVPlayer(playerItem: playerItem)
//: #. AVPlayerLayer
var playerLayer = AVPlayerLayer(player: player)
//: #. Frame for AVPlayerLayer
playerLayer.frame = playerView.bounds
//: #. Add AVPlayerLayer as a sublayer to a view
playerView.layer.addSublayer(playerLayer)

class ButtonHandler {
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

var playable: Playable = AVPlayerPlayable(with: player)
slider.seekTarget = playable
playable.addObserver(slider)
playable.addObserver(AdhocPlayableStateObserver({
    (playable, state, status) in
    if state == .playing {
        let image = status.playing ? #imageLiteral(resourceName: "icon-pause"): #imageLiteral(resourceName: "icon-play")
        button.setImage(image, for: .normal)
    }
    if state == .seeking {
        button.isEnabled = !status.seeking
    }

    if state == .currentTime {
        if playable.currentTime == playable.duration {
            playable.seek(to: kCMTimeZero)
        }
    }
}))

let buttonHandler = ButtonHandler(button: button)
buttonHandler.onTap = { (sender) in
    if playable.status.playing {
        player.pause()
    } else {
        player.play()
    }
}

player.play()

