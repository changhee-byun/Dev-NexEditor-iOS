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
    @State var showVideoPicker: Bool = false
    @State var showImagePicker: Bool = false
    @State var showPicker = [false, false]
    
    enum AssetType: Int {
        case Image = 0
        case Video = 1
    }
    
    let rows = [
        GridItem(.flexible())
    ]
    
    var body: some View {
        VStack {
            NexEditorEnginePreview(engineWrapper: self.editorEngineWrapper)
//                Spacer()
            ScrollView(.horizontal) {
                LazyHGrid(rows: self.rows, alignment: .top) {
//                    LazyVGrid(columns: columns) {
                    ForEach(0...10, id: \.self) { _ in
                        BeatTemplateAssetListCell()
                    }
                }
            }
        }
        .navigationBarTitle(Text("Beat template"), displayMode: .inline)
        .navigationBarItems(
            leading:
                Button(action: {
                    print("add user video")
                    showPicker(type: .Video)
                }) {
                    Image(systemName: "plus")
                }
                .sheet(isPresented: $showPicker[AssetType.Video.rawValue]) {
                    PhotoLibraryPicker(configuration: self.getPHPickerConfiguration(assetType: .Video), isPresented: $showPicker[AssetType.Video.rawValue]) { pickedAsset, assetName, thumbnail, fetchResult in
                        self.pickerCompletion(asset: pickedAsset, assetName: assetName, fetchResult: fetchResult)
                    }
                }
                ,
            trailing:
                Button(action: {
                    print("add user image")
                    showPicker(type: .Image)                    
                }) {
                    Image(systemName: "plus")
                }
                .sheet(isPresented: $showPicker[AssetType.Image.rawValue]) {
                    PhotoLibraryPicker(configuration: self.getPHPickerConfiguration(assetType: .Image), isPresented: $showPicker[AssetType.Image.rawValue]) { pickedAsset, assetName, thumbnail, fetchResult in
                        self.pickerCompletion(asset: pickedAsset, assetName: assetName, fetchResult: fetchResult)
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
    
    func showPicker(type: AssetType) {
        let authorizationStatus = PHPhotoLibrary.authorizationStatus()
        if authorizationStatus == .notDetermined {
            PHPhotoLibrary.requestAuthorization({status in
                if status == .authorized{
                    self.showPicker[type.rawValue].toggle()
                } else {}
            })
        }
        else if authorizationStatus == .authorized {
            self.showPicker[type.rawValue].toggle()
        }
    }
    
    func pickerCompletion(asset: Any, assetName: String, fetchResult: PHFetchResult<PHAsset>) {
        switch asset {
            case let asset as UIImage:
                print("image asset \(asset)")
                createProject(phFetchResult: fetchResult)
                
            case let asset as URL:
                print("asset name -> \(String(describing: assetName))")
                print("video asset \(asset)")
                
                createProject(phFetchResult: fetchResult)
                
                
            default:
                print("unsupported \(asset)")
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
    
    func getPHPickerConfiguration(assetType: AssetType)-> PHPickerConfiguration {
        var config = PHPickerConfiguration(photoLibrary: PHPhotoLibrary.shared())
        if assetType == .Image {
            config.filter = .images
        }
        else {
            config.filter = .videos
        }
        config.selectionLimit = 1
        return config
    }
    
}

struct BeatTemplateAssetListCell: View {
    var body: some View {
        Color.orange.frame(width: 60, height: 60)
    }
}

struct BeatTemplateMainView_Previews: PreviewProvider {
    static var previews: some View {
        BeatTemplateMainView()
    }
}
