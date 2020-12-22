//
//  DemoFeature.swift
//  EditorDemo
//
//  Created by CHANGHEE2 on 2020/12/23.
//

import Foundation


class DemoFeature {
    
    let title: String
    
    init(title: String) {
        self.title = title
    }
    
    convenience init() {
        self.init(title: "no title")
    }
}

extension DemoFeature: Hashable, Equatable {
    static func == (lhs: DemoFeature, rhs: DemoFeature) -> Bool {
        lhs.title == rhs.title
    }
    
    func hash(into hasher: inout Hasher) {
        hasher.combine(title)
    }
}
