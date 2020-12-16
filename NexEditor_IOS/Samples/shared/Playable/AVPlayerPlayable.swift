import Foundation
import AVFoundation

public class AVPlayerPlayable: NSObject, Playable {
    public var status: PlayableStatus { return _status }
    public var duration: CMTime { return player.currentItem?.duration ?? kCMTimeZero }
    public var currentTime: CMTime { return player.currentItem?.currentTime() ?? kCMTimeZero }
    public var observers: [PlayableStateObserver] = []
    
    @objc let player: AVPlayer
    
    private var kvoContext: Int = 0
    private var _status: ConcretePlayableStatus = ConcretePlayableStatus()
    
    private var timer: Timer!
    private var periodicTimeObserver: Any!

    public init(with player: AVPlayer) {
        self.player = player
        super.init()
        self.addObserver(self,
                         forKeyPath: #keyPath(AVPlayerPlayable.player.currentItem.duration),
                         options: [.initial, .new],
                         context: &kvoContext)
        
        if #available(iOS 10.0, *) {
            self.addObserver(self,
                             forKeyPath: #keyPath(AVPlayerPlayable.player.timeControlStatus),
                             options: [.initial, .new],
                             context: &kvoContext)
        } else {
            timer = Timer(timeInterval: 0.5, target: self, selector: #selector(timeout(_:)), userInfo: nil, repeats: true)
            RunLoop.current.add(timer, forMode: .defaultRunLoopMode)
        }
        
        periodicTimeObserver = player.addPeriodicTimeObserver(forInterval: CMTime(seconds: 0.5, preferredTimescale: 1000),
                                       queue: DispatchQueue.main)
        { [weak self] (time) in
            guard let me = self else { return }
            me.broadcastStateChange(.currentTime)
        }
    }
    
    deinit {
        if timer != nil {
            timer.invalidate()
        }
        self.removeObserver(self, forKeyPath:#keyPath(AVPlayerPlayable.player.currentItem.duration) )
        self.removeObserver(self, forKeyPath:#keyPath(AVPlayerPlayable.player.timeControlStatus) )
        player.removeTimeObserver(periodicTimeObserver)
    }
    
    // MARK: -
    @objc func timeout(_ sender: Timer) {
        let playing = player.rate > 0.0
        updatePlayingState(newState: playing)
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
            if let duration = change?[.newKey] as? CMTime, duration != kCMTimeIndefinite {
                self._status.loaded = true
                self.broadcastStateChange(.loaded)
            }
        }

        if #available(iOS 10.0, *) {
            if keyPath == #keyPath(AVPlayerPlayable.player.timeControlStatus) {
                if let status = change?[.newKey] as? NSNumber {
                    let playing = status.intValue == AVPlayerTimeControlStatus.playing.rawValue
                    self.updatePlayingState(newState: playing)
                }
            }
        } // otherwise, timeout(_:)
    }
    // MARK: - API
    public func play() {
        player.play()
    }
    
    public func pause() {
        player.pause()
    }
    
    public func changeState(_ state: PlayableStateChange, to value: Bool) {
        switch(state) {
        case .loaded:
            _status.loaded = value
            break
        case .playing:
            _status.playing = value
            break
        case .seeking:
            _status.seeking = value
            break
        default:
            break
        }
        broadcastStateChange(state)
    }
    
    var _waitingSeekTo: CMTime? = nil
    
    public func seek(to time: CMTime) {
        if _status.seeking {
            _waitingSeekTo = time
            return
        }
        _seek(to: time)
    }
    
    // MARK: - Internal
    func _seek(to time: CMTime) {
        _status.seeking = true
        player.seek(to: time, toleranceBefore: kCMTimeZero, toleranceAfter: kCMTimeZero) {
            [weak self] (finished) in
            
            guard let me = self else { return }
            me._status.seeking = false
            me.broadcastStateChange(.seeking)
            
            if let nextSeekTo = me._waitingSeekTo {
                me._waitingSeekTo = nil
                me.seek(to: nextSeekTo)
            }
        }
    }
    
    func updatePlayingState( newState playing: Bool ) {
        if _status.playing != playing {
            _status.playing = playing
            broadcastStateChange(.playing)
        }
    }

}

