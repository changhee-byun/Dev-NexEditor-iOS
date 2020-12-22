//
//  BeatTemplateMainView.swift
//  EditorDemo
//
//  Created by CHANGHEE2 on 2020/12/23.
//

import SwiftUI
import PhotosUI

struct BeatTemplateMainView: View {
    var editorEngineWrapper = NexEditorEngineWrapper()
    var project = NXEProject()
    @State fileprivate var asset:NXEBeatAssetItem?
    @State fileprivate var assetGroups:[NXEAssetItemGroup]!
    @State var showImagePicker: Bool = false
    
    var config: PHPickerConfiguration  {
        var config = PHPickerConfiguration(photoLibrary: PHPhotoLibrary.shared())
        config.filter = .videos
        config.selectionLimit = 0
        return config
    }
    
    var body: some View {
//        NavigationView {
            VStack {
//                Rectangle()
//                    .fill(Color.red)
//                    .frame(minWidth: 0,
//                           maxWidth: .infinity,
//                           minHeight: 0,
//                           maxHeight: .infinity)
//                HStack {
//                    Rectangle()
//                        .fill(Color.yellow)
//                        .frame(minWidth: 0,
//                               maxWidth: .infinity,
//                               minHeight: 0,
//                               maxHeight: .infinity)
//                    NexEditorEnginePreview(engineWrapper: self.editorEngineWrapper)
//                    Rectangle()
//                        .fill(Color.purple)
//                        .frame(minWidth: 0,
//                               maxWidth: .infinity,
//                               minHeight: 0,
//                               maxHeight: .infinity)
//                }
//                Rectangle()
//                    .fill(Color.blue)
//                    .frame(minWidth: 0,
//                           maxWidth: .infinity,
//                           minHeight: 0,
//                           maxHeight: .infinity)
                NexEditorEnginePreview(engineWrapper: self.editorEngineWrapper)
            }
//        }
        .navigationBarTitle(Text("Beat template"), displayMode: .inline)
        .navigationBarItems(
            trailing:
                Button(action: {
                    print("add user video")
                    showPicker()
                }) {
                    Image(systemName: "plus")
                }
            .sheet(isPresented: $showImagePicker) {
                PhotoLibraryPicker(configuration: self.config, isPresented: $showImagePicker) { pickedAsset, assetName, thumbnail, fetchResult in
                    switch pickedAsset {
                        case let asset as UIImage:
                            print("image asset \(asset)")
                            createProject(phFetchResult: fetchResult)
                            
                        case let asset as URL:
                            print("asset name -> \(String(describing: assetName))")
                            print("video asset \(asset)")
                            
                            createProject(phFetchResult: fetchResult)
                            
                            
                        default:
                            print("unsupported \(pickedAsset)")
                    }
                }
            }
        ) // end of navigationBarItems
        .onAppear() {
            loadBeatTemplate()
        }
        .onDisappear(){
            editorEngineWrapper.nxeEngine.stop()
        }
    }
    
    func showPicker() {
        let authorizationStatus = PHPhotoLibrary.authorizationStatus()
        if authorizationStatus == .notDetermined {
            PHPhotoLibrary.requestAuthorization({status in
                if status == .authorized{
                    self.showImagePicker.toggle()
                } else {}
            })
        }
        else if authorizationStatus == .authorized {
            self.showImagePicker.toggle()
        }
    }
    
    func loadBeatTemplate() {
        AssetPackageManager.manager.installAllPackages()
        self.assetGroups = NXEAssetLibrary.instance().groups(inCategory: NXEAssetItemCategory.beatTemplate)
        if let group = self.assetGroups.first, let asset = group.items[0] as? NXEBeatAssetItem {
            self.asset = asset
        }
        
    }
    
    func createProject(phFetchResult: PHFetchResult<PHAsset>) {
        var nexClipSources: [NXEClipSource?] = []
        
        for index in 0 ..< phFetchResult.count {
            nexClipSources.append(NXEClipSource(phAsset: phFetchResult[index]))
        }
        
        var clips : [NXEClip] = []
        for nexClipSource in nexClipSources {
            do {
                let clip = try NXEClip(source: nexClipSource)
                clips.append(clip)
            } catch {
                fatalError("Invalid clip")
            }
        }
        
        //project = NXEProject()
        project.visualClips = clips
        project.update()
        
        let duration = project.getTotalTime()
        var currentTime = self.editorEngineWrapper.nxeEngine.getCurrentPosition();
        editorEngineWrapper.nxeEngine.setProject(project);
        
        editorEngineWrapper.refreshPreview()
        
        
        editorEngineWrapper.nxeEngine.preparedEditor {
            editorEngineWrapper.nxeEngine.seek(0)
            editorEngineWrapper.nxeEngine.play()
        }
    }
    
}

struct BeatTemplateMainView_Previews: PreviewProvider {
    static var previews: some View {
        BeatTemplateMainView()
    }
}
