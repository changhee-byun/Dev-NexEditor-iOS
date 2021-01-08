import Foundation
import UIKit
import CoreMedia
import NexEditorFramework

//: ## Slider
//: - range: min/max    <- playable.loaded
//: - enabled status    <- playable.loaded, !seeking
//: - onSeek(_ time: CMTime) -> { }
/// Create an instance, add to a playable as an observer, and assign onSeek a block to seek playable whenever it slides.
public class UISeekSlider: UISlider, PlayableStateObserver {
    
    public var onSeek: ((_ time: CMTime)->Void)? = nil
    
    public func playable(_ playable: Playable, didChange state: PlayableStateChange, status: PlayableStatus) {
        self.isEnabled = status.loaded
        if state == .loaded {
            if status.loaded {
                self.minimumValue = 0
                self.maximumValue = Float(playable.duration.seconds)
                self.value = 0
            }
        }
        
        if state == .currentTime {
            self.value = Float(playable.currentTime.seconds)
        }
    }
    
    public override init(frame: CGRect) {
        super.init(frame: frame)
        commonInit()
    }
    
    public required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        commonInit()
    }
    
    func commonInit() {
        self.isEnabled = false
        addTarget(self, action: #selector(didSlide(_:)), for: .valueChanged)
    }
    
    @objc func didSlide(_ sender: UISlider) {
        let time = CMTime(seconds: Double(sender.value), preferredTimescale: 1000)
        onSeek?(time)
    }
}

