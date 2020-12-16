import Foundation
import CoreMedia

//: ## Playable
//: - Loading complete: loaded = true (ready to play)
//: - Properties
//:   - duration
//:   - currentTime
//: - States
//:   - loaded          default(false)
//:   - seeking         default(false)
//:   - playing         default(false=paused)
//: - Events
//:   - Loaded/Unloaded             changes loaded, duration/currentTime available
//:   - Playing/Paused              changes playing
//:   - didBeginSeek/didEndSeek     changes seeking
//:   - Current Time Changed        changes currentTime

public protocol PlayableStatus {
    var loaded: Bool { get }
    var playing: Bool { get }
    var seeking: Bool { get }
}

public enum PlayableStateChange {
    case loaded
    case playing
    case seeking
    case currentTime
}

public protocol PlayableStateObserver {
    func playable(_ playable: Playable, didChange state: PlayableStateChange, status: PlayableStatus)
}

public protocol Seekable {
    func seek(to: CMTime)
}

public protocol Playable: Seekable {
    var status: PlayableStatus { get }
    var duration: CMTime { get }
    var currentTime: CMTime { get }
    var observers: [PlayableStateObserver] { get set }
    
    func play()
}

public extension Playable {
    public mutating func addObserver(_ observer: PlayableStateObserver) {
        observers.append( observer )
    }
}

public protocol SeekControl: PlayableStateObserver {
    var seekTarget: Seekable? { get set}
}

public struct AdhocPlayableStateObserver: PlayableStateObserver {
    let changed: ( Playable, PlayableStateChange, PlayableStatus) -> Void
    
    public init(_ changed: @escaping ( Playable, PlayableStateChange, PlayableStatus) -> Void) {
        self.changed = changed
    }
    
    public func playable(_ playable: Playable, didChange state: PlayableStateChange, status: PlayableStatus) {
        changed(playable, state, status)
    }
}
