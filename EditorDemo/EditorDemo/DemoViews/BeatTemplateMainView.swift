//
//  BeatTemplateMainView.swift
//  EditorDemo
//
//  Created by CHANGHEE2 on 2020/12/23.
//

import SwiftUI
import PhotosUI
import NexEditorFramework

struct BeatTemplateMainView: View {
    @State var nexEditor = NexEditorExt()
    @State var nexEditorPlayer: Playable!
    
    @State var beatTemplates: [BeatTemplate] = []
    @State var selectedTemplate: BeatTemplate?
    
    @State var showPicker = [false, false]
    
    enum AssetType: Int {
        case Image = 0
        case Video = 1
    }
    
    let templateGridRow = [
        GridItem(.flexible(minimum: 32), spacing: 0)
    ]
    
    @State var currentTime: TimeInterval = TimeInterval()
    @State var duration: TimeInterval = TimeInterval()
    
    var body: some View {
        VStack {
            NexEditorView(engine: self.nexEditor)
                .onTapGesture {
                    self.nexEditorPlayer.playPause()
                }
            
            VStack {
                HStack {
                    Text(StringTools.formatTime(interval: self.currentTime))
                    Spacer()
                    Text(StringTools.formatTime(interval: self.duration))
                }
                ProgressBar(currentTime: self.$currentTime, duration: self.$duration).frame(height: 20)
            }
            //Color.primary.frame(width: .infinity, height: .infinity)
            
            // to do the work to show beat template assets.
            ScrollView(.horizontal) {
                LazyHGrid(rows: self.templateGridRow, alignment: .bottom, spacing: 0) {
                    ForEach(self.beatTemplates, id: \.self) { beatTemplate in
                        BeatTemplateAssetListCell(nexEditor: self.$nexEditor, template: beatTemplate) { selectedTemplate in
                            self.nexEditorPlayer.pause()
                            self.selectedTemplate = selectedTemplate
                            preparePlayback(template: self.selectedTemplate)
                        }
                        Spacer().frame(width:10)
                    }
                } // end of LazyHGrid
            }// end of ScrollView
            .frame(maxWidth: .infinity, maxHeight: 70)
        } // end of VStack
        .navigationBarTitle(Text("Beat template"), displayMode: .inline)
        .navigationBarItems(
            trailing:
                HStack {
                    Button(action: {
                        print("add user video")
                        showPicker(type: .Video)
                    }) {
                        Image(systemName: "video.badge.plus")
                    }
                    .sheet(isPresented: $showPicker[AssetType.Video.rawValue]) {
                        PhotoLibraryPicker(configuration: self.getPHPickerConfiguration(assetType: .Video), isPresented: $showPicker[AssetType.Video.rawValue], onPicked: self.pickerCompletion, onFetchResultPicked: self.pickerFetchResultCompletion(filter:fetchResult:))
                    }
                    Spacer()
                    Button(action: {
                        print("add user image")
                        showPicker(type: .Image)
                    }) {
                        Image(systemName: "photo.on.rectangle")
                    }
                    .sheet(isPresented: $showPicker[AssetType.Image.rawValue]) {
                        PhotoLibraryPicker(configuration: self.getPHPickerConfiguration(assetType: .Image), isPresented: $showPicker[AssetType.Image.rawValue], onPicked: self.pickerCompletion, onFetchResultPicked: self.pickerFetchResultCompletion(filter:fetchResult:))
                    }
                }
        ) // end of navigationBarItems
        .onAppear() {
            loadBeatTemplate()
            initPlayer()
        }
        .onDisappear(){
            self.nexEditorPlayer.pause()
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
                
            case let asset as URL:
                print("asset name -> \(String(describing: assetName))")
                print("video asset \(asset)")
                
            default:
                print("unsupported \(asset)")
        }
    }
    
    func pickerFetchResultCompletion(filter: PHPickerFilter, fetchResult: PHFetchResult<PHAsset>) {
        switch filter {
            case .images:
                prepareClipSource(phFetchResult: fetchResult)
                preparePlayback(template: self.selectedTemplate)
            case .videos:
                prepareClipSource(phFetchResult: fetchResult)
                preparePlayback(template: self.selectedTemplate)
            default:
                print("unsupported picker result")
        }
    }
    
    func loadBeatTemplate() {
        self.beatTemplates = self.nexEditor.loadBeatTemplates()
        
        if self.beatTemplates.count != 0 {
            self.selectedTemplate = self.beatTemplates[0]
        }
    }
    
    func initPlayer() {
        nexEditorPlayer = NexEditorPlayer(nexEditorExt: self.nexEditor)
        //playable.addObserver(slider)
        nexEditorPlayer.addObserver(AdhocPlayableStateObserver({(playable, state, status) in
            if state == .playing {
//                let imageNameDefault = status.playing ? "pause_default" : "play_default"
//                self?.playPauseButton.setImage(UIImage(named:imageNameDefault), for: .normal)
            }
            if state == .loaded {
//                self?.playPauseButton.isEnabled = status.loaded
                self.duration = TimeInterval(playable.duration.seconds)
                self.currentTime = status.loaded ? 0 : playable.currentTime.seconds
                self.nexEditorPlayer.play()
            }
            if state == .currentTime {
                self.currentTime = playable.currentTime.seconds
            }
        }))
    }
    
    func prepareClipSource(phFetchResult: PHFetchResult<PHAsset>) {
        _ = self.nexEditor.setClips(phFetchResult: phFetchResult)
    }
    
    func preparePlayback(template: BeatTemplate?) -> Void {
        guard let asset = template else {
            NSLog("empty beat template asset")
            return
        }
        _ = self.nexEditor.setBeatTemplate(asset)
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
    @Binding var nexEditor: NexEditorExt
    let template: BeatTemplate
    var selectCompletion: ((_ item: BeatTemplate) -> Void)
    
    @State var assetThumbnail: UIImage? = nil
    
    
    var body: some View {
        ZStack {
            Color.yellow.frame(width: 70, height: 70)
                .onTapGesture {
                    NSLog("tap item")
                }
                
            if let thumb = assetThumbnail {
                Image(uiImage: thumb)
                    .onTapGesture {
                        selectCompletion(self.template)
                    }
            }
        }
        .onAppear() {
            loadThumb()
        }
    
    }
    
    func loadThumb() {
        self.nexEditor.getThumbnail(template: self.template, size: CGSize(width: 60, height: 60)) { image in
            self.assetThumbnail = image
        }
    }
}

struct BeatTemplateMainView_Previews: PreviewProvider {
    static var previews: some View {
        BeatTemplateMainView()
    }
}
