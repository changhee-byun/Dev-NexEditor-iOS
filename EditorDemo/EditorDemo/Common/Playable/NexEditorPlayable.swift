//
//  NexEditorPlayable.swift
//  QuickPrototypeVideoPlayer
//
//  Created by Simon Kim on 11/22/17.
//  Copyright © 2017 NexStreaming Crop. All rights reserved.
//

import Foundation
import CoreMedia
import NexEditorFramework

extension CMTime {
    init(timeMs: Int32) {
        self.init(seconds: Double(timeMs) / 1000.0, preferredTimescale: 1000)
    }
}

/*
 *  Events:
 *      loaded/unloaded
 *      playing/paused
 *      seeking/end
 *      current time changed
 *
 */
public class NexEditorPlayable: Playable {
    
    public var status: PlayableStatus { return _status }
    public var duration: CMTime { return CMTime(timeMs: editor.project?.getTotalTime() ?? 0) }
    public var currentTime: CMTime { return CMTime(timeMs: editor.getCurrentPosition()) }
    public var observers: [PlayableStateObserver] = []
    private var _status: ConcretePlayableStatus = ConcretePlayableStatus()

    private let editor: NXEEngine
    
    init(editor: NXEEngine) {
        self.editor = editor
        editor.setCallbackWithPlayProgress({ [weak self] (time) in
            self?.broadcastStateChange(.currentTime)
        }, playStart: { [weak self] in
            self?._status.playing = true
            self?.broadcastStateChange(.playing)
        }) { [weak self] in
            self?._status.playing = false
            self?.broadcastStateChange(.playing)
        }
        
        editor.setCallbackWithSetTime { [weak self] (seekTime) in
            self?._status.seeking = false
            self?.broadcastStateChange(.seeking)
            self?.broadcastStateChange(.currentTime)
        }
    }
    
    deinit {
        
    }
    
    public func play() {
        if !self._status.playing {
            editor.resume()
            self._status.playing = true
            self.broadcastStateChange(.playing)
        }
    }
    
    public func pause() {
        if self._status.playing {
            editor.pause()
            self._status.playing = false
            self.broadcastStateChange(.playing)
        }
    }
    
    public func playPause() {
        if !self._status.playing {
            play()
        }
        else {
            pause()
        }
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

    public func seek(to time: CMTime) {
        pause()
        editor.seek(Int32(time.seconds * 1000))
        if !_status.seeking {
            _status.seeking = true
            self.broadcastStateChange(.seeking)
        }
    }

}
