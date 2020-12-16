//
//  EditorTesterApp.swift
//  EditorTester
//
//  Created by ERIC0 on 2020/10/22.
//

import SwiftUI

@main
struct EditorTesterApp: App {
    @Environment(\.scenePhase) private var scenePhase
    var engineManager = EngineManager()
    
    var body: some Scene {
        WindowGroup {
            ContentView(engineMan: engineManager)
        }
        .onChange(of: scenePhase) { newScenePhase in
            switch(newScenePhase) {
            case .active :
//                engineManager.makeReady()

                print("active")
            case .inactive :
                print("inactive")
            case .background :
                print("background")
            @unknown default:
                print("unknown")
            }
        }
    }
}
