//
//  ButtonTarget.swift
//  QuickPrototypeVideoPlayer
//
//  Created by Simon Kim on 11/22/17.
//  Copyright © 2017 NexStreaming Crop. All rights reserved.
//

import Foundation
import UIKit

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
