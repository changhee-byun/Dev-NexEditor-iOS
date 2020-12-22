//
//  ContentView.swift
//  EditorTester
//
//  Created by ERIC0 on 2020/10/22.
//

import SwiftUI
import PhotosUI

struct ContentView: View {
    
    var engineMan : EngineManager
    //@State var videoMode: Bool = false
    @State private var isPresented1: Bool = false
    @State private var isPresented2: Bool = false
    //@State var pickerResult: [UIImage] = []
    @State var clipSources: [NXEClipSource?] = []
    var project = NXEProject()
    
//    mutating func changeModeToVideo(video:Bool) {
//        self.videoMode = video
//    }
    
    var configImages: PHPickerConfiguration  {
        var config = PHPickerConfiguration(photoLibrary: PHPhotoLibrary.shared())
        config.filter = .images //videos, livePhotos...
        config.selectionLimit = 0 //0 => any, set 1-2-3 for har limit
        
        return config
    }
    
    var configVideo: PHPickerConfiguration  {
        var config = PHPickerConfiguration(photoLibrary: PHPhotoLibrary.shared())
        config.filter = .videos
        config.selectionLimit = 1 //0 => any, set 1-2-3 for har limit
        
        return config
    }

    var body: some View {
        VStack {
            EngineView(engineMan: engineMan)
            
            HStack {
                VStack {
                    Button("Select Video") {
                        //self.changeModeToVideo(video: true)
                        isPresented2.toggle()
                        
                    }
                    Button("Select Photos") {
                        //changeModeToVideo(video: false)
                        isPresented1.toggle()
                    }
                }
                .sheet(isPresented: $isPresented2) {
                    PhotoPickerView(configuration: configVideo,
                                    //pickerResult: $pickerResult,
                                    pickerResult: $clipSources,
                                    isPresented: $isPresented2)
                }
                .sheet(isPresented: $isPresented1) {
                    PhotoPickerView(configuration: configImages,
                                    //pickerResult: $pickerResult,
                                    pickerResult: $clipSources,
                                    isPresented: $isPresented1)
                }
                

                Button("Make Project") {
                    
                    var clips : [NXEClip] = []
                    for clipSource in clipSources {
                        do {
                            let clip = try NXEClip(source: clipSource)
                            clips.append(clip)
                        } catch {
                            
                        }
                    }
                    
                    project.visualClips = clips
                    project.update()
                    
                    var duration = project.getTotalTime()
                    var currentTime = engineMan.engine?.getCurrentPosition();
                    engineMan.engine?.setProject(project);

                    engineMan.engine?.preparedEditor {
                        engineMan.engine?.seek(0)
                    }
                }
                Button("Play") {
                    engineMan.engine?.play()
                }
                //)
            }
        }
        .onAppear {
            engineMan.makeReady()
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        //Text("Only Test")
        ContentView(engineMan:EngineManager())
    }
}
