//import Foundation
//import CoreMedia
//
////: ## Playable
////: - Loading complete: loaded = true (ready to play)
////: - Properties
////:   - duration
////:   - currentTime
////: - States
////:   - loaded          default(false)
////:   - seeking         default(false)
////:   - playing         default(false=paused)
////: - Events
////:   - Loaded/Unloaded             changes loaded, duration/currentTime available
////:   - Playing/Paused              changes playing
////:   - didBeginSeek/didEndSeek     changes seeking
////:   - Current Time Changed        changes currentTime
//
//public protocol PlayableStatus {
//    var loaded: Bool { get }
//    var playing: Bool { get }
//    var seeking: Bool { get }
//}
//
//public enum PlayableStateChange {
//    case loaded
//    case playing
//    case seeking
//    case currentTime
//}
//
//public protocol PlayableStateObserver {
//    func playable(_ playable: Playable, didChange state: PlayableStateChange, status: PlayableStatus)
//}
//
//public protocol Playable {
//    var status: PlayableStatus { get }
//    var duration: CMTime { get }
//    var currentTime: CMTime { get }
//    var observers: [PlayableStateObserver] { get set }
//    
//    func play()
//    func pause()
//    func playPause()
//    func seek(to: CMTime)
//    func changeState(_ state: PlayableStateChange, to value: Bool)
//}
//
//public extension Playable {
//    public mutating func addObserver(_ observer: PlayableStateObserver) {
//        observers.append( observer )
//    }
//    
//    public func broadcastStateChange(_ change: PlayableStateChange) {
//        for observer in observers {
//            observer.playable(self, didChange: change, status: self.status)
//        }
//    }
//}
//
//public struct AdhocPlayableStateObserver: PlayableStateObserver {
//    let changed: ( Playable, PlayableStateChange, PlayableStatus) -> Void
//    
//    public init(_ changed: @escaping ( Playable, PlayableStateChange, PlayableStatus) -> Void) {
//        self.changed = changed
//    }
//    
//    public func playable(_ playable: Playable, didChange state: PlayableStateChange, status: PlayableStatus) {
//        changed(playable, state, status)
//    }
//}
//
//struct ConcretePlayableStatus: PlayableStatus {
//    var loaded: Bool = false
//    var playing: Bool = false
//    var seeking: Bool = false
//}
