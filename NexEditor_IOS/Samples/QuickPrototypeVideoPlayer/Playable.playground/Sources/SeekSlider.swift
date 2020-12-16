import Foundation
import UIKit
import CoreMedia

//: ## Slider
//: - range: min/max    <- playable.loaded
//: - enabled status    <- playable.loaded, !seeking
//: - seek -> seekTarget.seek(to: slider.value)

public class SeekSlider: UISlider, SeekControl {
    public var seekTarget: Seekable?
    
    public func playable(_ playable: Playable, didChange state: PlayableStateChange, status: PlayableStatus) {
        self.isEnabled = status.loaded
        if state == .loaded {
            if status.loaded {
                self.minimumValue = 0
                self.maximumValue = Float(playable.duration.seconds)
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
        seekTarget?.seek(to: time)
    }
}

