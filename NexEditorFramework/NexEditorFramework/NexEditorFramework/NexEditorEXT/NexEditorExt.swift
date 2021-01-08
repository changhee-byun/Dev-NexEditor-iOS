//
//  NexEditorFrameworkWrapper.swift
//  NexEditorFramework
//
//  Created by CHANGHEE2 on 2021/01/07.
//  Copyright © 2021 NexStreaming Corp. All rights reserved.
//

import Foundation

public class NexEditorExt {
    public var nxeEngine : NXEEngine = NXEEngine()
    public var preview : UIView = UIView()
    public var clips : [NXEClip]? = nil
    public var beatTemplates: [BeatTemplate] = []
    
    var assetGroups: [NXEAssetItemGroup] = []
    var nxeBeatAssets: [NXEBeatAssetItem] = []
    var project: NXEProject!
    
    public init(engine: NXEEngine) {
        self.nxeEngine = engine
        setDefault()
    }
    
    public convenience init() {
        self.init(engine: NXEEngine())
        self.clips = nil
    }
    
    public func setDefault() {
        //enableEditorLog(true)
        self.nxeEngine.aspectType = .ratio9v16
        self.nxeEngine.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
    }
    
    public func refreshPreview() {
        DispatchQueue.main.async {
            self.nxeEngine.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
        }
    }
    
    public func refreshEngine() {
        nxeEngine = NXEEngine()
        setDefault()
    }
    
//    public func enableEditorLog(_ enabled:Bool) {
//        let level = enabled ? 0 : 4
////        NexEditorLog.setLogLevel(level, for: .err)
////        NexEditorLog.setLogLevel(level, for: .warning)
////        NexEditorLog.setLogLevel(level, for: .info)
////        NexEditorLog.setLogLevel(level, for: .debug)
//    }
    
    public func setClips(phFetchResult sources: PHFetchResult<PHAsset>)-> Bool {
        var phAssetSources: [PHAsset] = []
        
        for index in 0 ..< sources.count {
            phAssetSources.append(sources[index])
        }
        
        return self.setClips(phAssets: phAssetSources)
    }
    
    public func setClips(phAssets sources: [PHAsset])-> Bool {
        return self.prepareClipSources(userSources: sources)
    }
    
    public func setClips(avUrlAssets sources: [AVURLAsset])-> Bool {
        return self.prepareClipSources(userSources: sources)
    }
    
    public func setClips(path sources: [NSString])-> Bool {
        return self.prepareClipSources(userSources: sources)
    }
    
    public func setClips(mediaItem sources: [MPMediaItem])-> Bool {
        return self.prepareClipSources(userSources: sources)
    }
    
    public func loadBeatTemplates()-> [BeatTemplate] {
        NexEditorAssetPackageManager.manager.installAllPackages()
        self.assetGroups = NXEAssetLibrary.instance().groups(inCategory: NXEAssetItemCategory.beatTemplate)
        self.nxeBeatAssets = self.assetGroups.map { $0.items[0] as? NXEBeatAssetItem }.compactMap { $0 }
        
        
        self.beatTemplates = self.nxeBeatAssets.map {
            print($0)
            //return BeatTemplate(audioId: $0.audioId, title: $0.title, thumbnailHandler: NXEBeatAssetItem.loadThumbnail($0))
            return BeatTemplate(audioId: $0.audioId, title: $0.title)
        }

        return self.beatTemplates
    }
    
    public func setBeatTemplate(_ beatTemplate: BeatTemplate)-> Bool {
        guard let selectedAsset = self.getNxeBeatAsset(beatTemplate: beatTemplate) else {
            return false
        }
        
        do {
            
            self.project = try NXEBeatTemplateProject(beatTemplateAssetItem: selectedAsset, clips: self.clips)
            self.nxeEngine.setProject(self.project)
            
            refreshPreview()
            
            self.nxeEngine.preparedEditor {
                //                playable.seek(to: CMTime.zero)
                //                playable.changeState(.loaded, to: true)
                self.nxeEngine.play()
                //                editorEngineWrapper.nxeEngine.stop()
                //                editorEngineWrapper.nxeEngine.seek(0)
                //                let ret = editorEngineWrapper.nxeEngine.play()
                //NSLog("play -> \(ret)")
            }

            
        }
        catch let e {
            print(e.localizedDescription)
            return false
        }
        
        return true
    }
    
    public func getThumbnail(template: BeatTemplate, size: CGSize, _ loaded: @escaping (_ image: UIImage?)->Void) {
        guard let selectedAsset = self.getNxeBeatAsset(beatTemplate: template) else {
            loaded(nil)
            return
        }
        
        selectedAsset.loadThumbnail(size: size, loaded)
    }
}

extension NexEditorExt {
    func prepareClipSources(userSources: [Any])-> Bool {
        var nexClipSources: [NXEClipSource] = []
        
        userSources.forEach {
            switch $0 {
                case let asset as PHAsset:
                    nexClipSources.append(NXEClipSource(phAsset: asset))
                case let asset as AVURLAsset:
                    nexClipSources.append(NXEClipSource(avurlAsset: asset))
                case let asset as NSString:
                    nexClipSources.append(NXEClipSource(path: asset as String))
                case let asset as MPMediaItem:
                    nexClipSources.append(NXEClipSource(mediaItem: asset))
                default :
                    NSLog("Invalid clip source - \($0)")
            }
        }
        
        var clips : [NXEClip] = []
        for nexClipSource in nexClipSources {
            do {
                let clip = try NXEClip(source: nexClipSource)
                clips.append(clip)
            } catch {
                NSLog("Invalid clip")
            }
        }
        
        if clips.count == nexClipSources.count {
            self.clips = clips
            return true
        }
        else {
            return false
        }
    }
    
    func getNxeBeatAsset(beatTemplate: BeatTemplate)-> NXEBeatAssetItem? {
        return self.nxeBeatAssets.filter({$0.audioId == beatTemplate.audioId}).first
    }
}
