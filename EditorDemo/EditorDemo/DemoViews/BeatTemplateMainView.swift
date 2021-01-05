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
//    @State var project: NXEProject!
    @State fileprivate var editor: NXEEngine!
    @State fileprivate var project: NXEProject!
    @State fileprivate var playable: Playable!
    @State fileprivate var clips : [NXEClip]? = nil
    
    @State var selectedTemplate: NXEBeatAssetItem?
    @State var assetGroups: [NXEAssetItemGroup] = []
    @State var showPicker = [false, false]
    
    enum AssetType: Int {
        case Image = 0
        case Video = 1
    }
    
    let rows = [
        GridItem(.flexible(minimum: 32), spacing: 0)
    ]
    
    var body: some View {
        VStack {
            NexEditorEnginePreview(engineWrapper: self.editorEngineWrapper)
            
            //Color.primary.frame(width: .infinity, height: .infinity)

            // to do the work to show beat template assets.
            ScrollView(.horizontal) {
                LazyHGrid(rows: self.rows, alignment: .bottom, spacing: 0) {
                    ForEach(self.assetGroups, id: \.self) { asset in
                        BeatTemplateAssetListCell(asset: asset.items[0] as? NXEBeatAssetItem) { asset in
                            editorEngineWrapper.nxeEngine.stop()
                            self.selectedTemplate = asset
                            preparePlayback(asset: self.selectedTemplate)
                        }
                    }
                } // end of LazyHGrid
            }// end of ScrollView
            .frame(maxWidth: .infinity, maxHeight: 70)
        } // end of VStack
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
                    PhotoLibraryPicker(configuration: self.getPHPickerConfiguration(assetType: .Video), isPresented: $showPicker[AssetType.Video.rawValue], onPicked: self.pickerCompletion, onFetchResultPicked: self.pickerFetchResultCompletion(filter:fetchResult:))
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
                    PhotoLibraryPicker(configuration: self.getPHPickerConfiguration(assetType: .Image), isPresented: $showPicker[AssetType.Image.rawValue], onPicked: self.pickerCompletion, onFetchResultPicked: self.pickerFetchResultCompletion(filter:fetchResult:))
                }
        ) // end of navigationBarItems
        .onAppear() {
            editor = NXEEngine.instance()
            loadBeatTemplate()
            initPlayer()
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
    
    func pickerCompletion(asset: Any, assetName: String, thumbNail: UIImage) {
        switch asset {
            case let asset as UIImage:
                print("image asset \(asset)")
                //createProject(phFetchResult: fetchResult)
                
            case let asset as URL:
                print("asset name -> \(String(describing: assetName))")
                print("video asset \(asset)")
                //createProject(phFetchResult: fetchResult)
                
            default:
                print("unsupported \(asset)")
        }
    }
    
    func pickerFetchResultCompletion(filter: PHPickerFilter, fetchResult: PHFetchResult<PHAsset>) {
        switch filter {
            case .images:
                prepareClipSource(phFetchResult: fetchResult)
                preparePlayback(asset: self.selectedTemplate)
            case .videos:
                prepareClipSource(phFetchResult: fetchResult)
                preparePlayback(asset: self.selectedTemplate)
            default:
                print("unsupported picker result")
        }
    }
    
    func loadBeatTemplate() {
        AssetPackageManager.manager.installAllPackages()
        self.assetGroups = NXEAssetLibrary.instance().groups(inCategory: NXEAssetItemCategory.beatTemplate)
        if let group = self.assetGroups.first, let asset = group.items[0] as? NXEBeatAssetItem {
            self.selectedTemplate = asset
        }
        
//        self.assetGroups.forEach {
//            let thumbnailLoadable = $0 as ThumbnailLoadable
//            let size = CGSize(width: 90, height: 90)
//            thumbnailLoadable.loadThumbnail(size: size) { image in
//                image
//            }
//        }
        
    }
    
    func initPlayer() {
        playable = NexEditorPlayable(editor: editor)
//        playable.addObserver(slider)
        playable.addObserver(AdhocPlayableStateObserver({(playable, state, status) in
            if state == .playing {
//                let imageNameDefault = status.playing ? "pause_default" : "play_default"
//                self?.playPauseButton.setImage(UIImage(named:imageNameDefault), for: .normal)
            }
            if state == .loaded {
//                self?.playPauseButton.isEnabled = status.loaded
//                self?.contentDuration.text = StringTools.formatTime(interval: TimeInterval(playable.duration.seconds))
//                let time = status.loaded ? 0 : playable.currentTime.seconds
//                self?.currentTime.text = StringTools.formatTime(interval: time)
            }
            if state == .currentTime {
//                self?.currentTime.text = StringTools.formatTime(interval: TimeInterval(playable.currentTime.seconds))
            }
        }))
        
        //self.preparePlayback(editor: editor, asset: asset)
    }
    
    func prepareClipSource(phFetchResult: PHFetchResult<PHAsset>) {
        var nexClipSources: [NXEClipSource] = []
        
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
        
        self.clips = clips
    }
    
    func preparePlayback(asset:NXEBeatAssetItem?) -> Void {
        guard let clips = self.clips, let asset = self.selectedTemplate else {
            NSLog("Empty clip sources. or empty beat template asset")
            return
        }
        
        do {
            self.assetGroups = NXEAssetLibrary.instance().groups(inCategory: NXEAssetItemCategory.beatTemplate)
            
            project = try NXEBeatTemplateProject(beatTemplateAssetItem: asset, clips:clips)
            editorEngineWrapper.nxeEngine.setProject(project);
            
            editorEngineWrapper.refreshPreview()
            
            
            editorEngineWrapper.nxeEngine.preparedEditor {
                editorEngineWrapper.nxeEngine.stop()
                editorEngineWrapper.nxeEngine.seek(0)
                let ret = editorEngineWrapper.nxeEngine.play()
                NSLog("play -> \(ret)")
            }
            
            playable.seek(to: CMTime.zero)
            playable.changeState(.loaded, to: true)
        }
        catch let e {
            print(e.localizedDescription)
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
        
        project = NXEProject()
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
        config.selectionLimit = 0
        return config
    }
    
}

struct BeatTemplateAssetListCell: View {
    @State var assetThumbnail: UIImage? = nil
    let asset: NXEBeatAssetItem?
    var selectCompletion: ((_ item: NXEBeatAssetItem) -> Void)
    
    var body: some View {
        ZStack {
            Color.orange.frame(width: 70, height: 70)
                .onTapGesture {
                    NSLog("tap item")
                }
                
            if let thumb = assetThumbnail {
                Image(uiImage: thumb)
                    .onTapGesture {
                        if asset != nil {
                            selectCompletion(self.asset!)
                        }
                    }
            }
        }
        .onAppear() {
            loadThumb(thumbnail: self.asset)
        }
    
    }
    
    func loadThumb(thumbnail: ThumbnailLoadable?) {
        guard let thumb = thumbnail else {
            return
        }
        
        thumb.loadThumbnail(size: CGSize(width: 60, height: 60)) { image in
            self.assetThumbnail = image
        }
    }
}

struct BeatTemplateMainView_Previews: PreviewProvider {
    static var previews: some View {
        BeatTemplateMainView()
    }
}
