import Foundation
import AVFoundation

struct AVPlayerPlayableState: PlayableStatus {
    var loaded: Bool = false
    var playing: Bool = false
    var seeking: Bool = false
}

public class AVPlayerPlayable: NSObject, Playable {
    public var status: PlayableStatus { return _state }
    public var duration: CMTime { return player.currentItem?.duration ?? kCMTimeZero }
    public var currentTime: CMTime { return player.currentItem?.currentTime() ?? kCMTimeZero }
    public var observers: [PlayableStateObserver] = []
    
    @objc let player: AVPlayer
    
    private var kvoContext: Int = 0
    private var _state: AVPlayerPlayableState = AVPlayerPlayableState()
    
    public init(with player: AVPlayer) {
        self.player = player
        super.init()
        self.addObserver(self,
                         forKeyPath: #keyPath(AVPlayerPlayable.player.currentItem.duration),
                         options: [.initial, .new],
                         context: &kvoContext)
        self.addObserver(self,
                         forKeyPath: #keyPath(AVPlayerPlayable.player.timeControlStatus),
                         options: [.initial, .new],
                         context: &kvoContext)
        
        player.addPeriodicTimeObserver(forInterval: CMTime(seconds: 0.5, preferredTimescale: 1000),
                                       queue: DispatchQueue.main)
        { [weak self] (time) in
            guard let me = self else { return }
            me.broadcastStateChange(.currentTime)
        }
    }
    
    deinit {
        self.removeObserver(self, forKeyPath:#keyPath(AVPlayerPlayable.player.currentItem.duration) )
        self.removeObserver(self, forKeyPath:#keyPath(AVPlayerPlayable.player.timeControlStatus) )
    }
    
    override public func observeValue(forKeyPath keyPath: String?,
                                      of object: Any?,
                                      change: [NSKeyValueChangeKey : Any]?,
                                      context: UnsafeMutableRawPointer?) {
        // Only handle observations for the playerItemContext
        guard context == &kvoContext else {
            super.observeValue(forKeyPath: keyPath,
                               of: object,
                               change: change,
                               context: context)
            return
        }
        if keyPath == #keyPath(AVPlayerPlayable.player.currentItem.duration) {
            if change?[.newKey] != nil {
                self._state.loaded = true
                self.broadcastStateChange(.loaded)
            }
        }
        
        if keyPath == #keyPath(AVPlayerPlayable.player.timeControlStatus) {
            if let status = change?[.newKey] as? NSNumber {
                self._state.playing = status.intValue == AVPlayerTimeControlStatus.playing.rawValue
                self.broadcastStateChange(.playing)
            }
        }
    }
    
    public func play() {
        player.play()
    }
    
    var _waitingSeekTo: CMTime? = nil
    
    public func seek(to time: CMTime) {
        if _state.seeking {
            _waitingSeekTo = time
            return
        }
        _seek(to: time)
    }
    
    
    func _seek(to time: CMTime) {
        _state.seeking = true
        player.seek(to: time, toleranceBefore: kCMTimeZero, toleranceAfter: kCMTimeZero) {
            [weak self] (finished) in
            
            guard let me = self else { return }
            me._state.seeking = false
            me.broadcastStateChange(.seeking)
            
            if let nextSeekTo = me._waitingSeekTo {
                me._waitingSeekTo = nil
                me.seek(to: nextSeekTo)
            }
        }
    }
    
    func broadcastStateChange(_ change: PlayableStateChange) {
        for observer in observers {
            observer.playable(self, didChange: change, status: self.status)
        }
    }
}

