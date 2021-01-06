//
//  ProgressBar.swift
//  EditorDemo
//
//  Created by CHANGHEE2 on 2021/01/05.
//

import SwiftUI

struct ProgressBar: View {
    @Binding var currentTime: TimeInterval
    @Binding var duration: TimeInterval
    
    var body: some View {
        GeometryReader { geometry in
            ZStack(alignment: .leading) {
                Rectangle().frame(width: geometry.size.width , height: geometry.size.height)
                    .opacity(0.3)
                    .foregroundColor(Color(UIColor.systemTeal))
                
                Rectangle().frame(width: self.getRelativeProgress(value: self.currentTime, geometrySize: geometry.size.width), height: geometry.size.height)
                    .foregroundColor(Color(UIColor.systemBlue))
                    .animation(.linear)
            }.cornerRadius(45.0)
        }
    }
    
    func getRelativeProgress(value: TimeInterval, geometrySize: CGFloat) -> CGFloat {
        return min((CGFloat(value) * geometrySize) / CGFloat(self.duration), geometrySize)
    }
}
