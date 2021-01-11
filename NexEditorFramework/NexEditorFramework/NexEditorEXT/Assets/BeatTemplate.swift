//
//  BeatTemplate.swift
//  NexEditorFramework
//
//  Created by CHANGHEE2 on 2021/01/08.
//  Copyright © 2021 NexStreaming Corp. All rights reserved.
//

import Foundation

public struct BeatTemplate: Hashable {
    public let audioId: String
    public let title: String
    
    public init() {
        self.init(audioId: "test", title: "title")
    }
    
    public init(audioId: String, title: String) {
        self.audioId = audioId
        self.title = title
    }
    
    public func hash(into hasher: inout Hasher) {
        hasher.combine(audioId)
    }
}
