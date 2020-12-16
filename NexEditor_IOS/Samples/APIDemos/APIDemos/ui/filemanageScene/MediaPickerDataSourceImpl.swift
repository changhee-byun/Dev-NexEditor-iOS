/**
 * File Name   : MediaPickerDataSourceImpl.swift
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
 *                         http://www.nexstreaming.com
 *
 *******************************************************************************
 *     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *     PURPOSE.
 *******************************************************************************
 *
 */

import Foundation

class MediaPickerItemData: ItemPickerItem {
    
    private var image: UIImage?
    private var mediaItem: ClipMediaItem
    
    var title: String
    var desc: String? {
        var result:String = ""
        if(mediaItem.type == .video) {
            if let avurlAsset = mediaItem.avurlAsset {
                let videoTracks = avurlAsset.tracks(withMediaType: AVMediaType.video)
                let height = videoTracks.first?.naturalSize.height ?? 0
                let width = videoTracks.first?.naturalSize.width ?? 0
                let bps = videoTracks.first?.estimatedDataRate ?? 0
                let fps = videoTracks.first?.nominalFrameRate ?? 0
                result = String(format: "%.0f x %.0f %0.f fps \nBitrate: %@", width, height, fps, StringTools.humanReadableBits(Int(bps)))
            }
        }
        else if(mediaItem.type == .image) {
            var resolution: (width: Int, height: Int)? = nil
            if mediaItem.phAsset != nil {
                resolution = (Int(mediaItem.phAsset.pixelWidth), Int(mediaItem.phAsset.pixelHeight))
            }
            else if(mediaItem.filePath != nil) {
                if let img = UIImage(contentsOfFile: mediaItem.filePath) {
                    resolution = (Int(img.size.width), Int(img.size.height))
                }
            }
            if let resolution = resolution {
                result = String(format: "%d x %d", resolution.width, resolution.height)
            }
        }
        return result
    }

    func loadImage(size: CGSize, loaded: @escaping (UIImage?)->Void ) {
        mediaItem.loadThumbnail(size: size) { (image) in
            self.image = image
            loaded(image)
        }
    }
    
    init(title: String, mediaItem: ClipMediaItem) {
        self.title = title
        self.mediaItem = mediaItem
    }
}

struct MediaPickerSelection {
    var mediaItems: [ClipMediaItem] = []
    
    var clipSources: [NXEClipSource] {
        return mediaItems.flatMap({ $0.clipSource })
    }
}

class MediaPickerDataSourceImpl: NSObject, MediaPickerDataSource {
    let options: MediaPickerOptions
    private var mediaSource: FileManagerDataController!
    
    lazy var types: [ClipMediaType] = {
        var result: [ClipMediaType] = []
        if self.options.video { result.append(.video) }
        if self.options.image { result.append(.image) }
        if self.options.audio { result.append(.audio) }
        return result
    }()
    
    // MAKR: - ItemPickerDataSource
    var sectionCount: Int {
        return sectionTitles.count
    }
    
    var sectionTitles: [String] {
        return types.map{ $0.displayText }
    }
    
    func loadItems(_ loaded: @escaping ()->Void) {
        let dq = DispatchQueue.global()
        dq.async {
            self.mediaSource = FileManagerDataController(video: self.options.video,
                                                         image: self.options.image,
                                                         audio: self.options.audio)
            loaded()
        }
    }
    
    func itemCount(in section: Int) -> Int {
        let type = types[section]
        return Int(mediaSource?.items(of: type).count ?? 0)
    }
    
    var itemsCache: [IndexPath: MediaPickerItemData] = [:]
    
    func item(at indexPath:IndexPath) -> ItemPickerItem {
        
        if let item = itemsCache[indexPath] {
            return item
        }
        
        let index = Int32(indexPath.row)
        let type = types[indexPath.section]
        
        var title: String = ""
        
        let mediaItem = mediaSource.item(for: type, at: Int(index))!
        
        if mediaItem.phAsset != nil {
            title = String(format:"IMG_%03d", index)
        } else if mediaItem.filePath != nil {
            title = (mediaItem.filePath as NSString).lastPathComponent
        }
        let result = MediaPickerItemData(title: title, mediaItem: mediaItem)
        
        itemsCache[indexPath] = result
        return result
    }

    func shouldShowInfoButton(at indexPath: IndexPath) -> Bool {
        return self.options.showInfoButton
    }
    
    // MARK: -
    func selection(with rows: [IndexPath]) -> MediaPickerSelection {
        var items: [ClipMediaItem] = []
        
        if let mediaSource = mediaSource {
            for indexPath in rows {
                let type = types[indexPath.section]
                if let item = mediaSource.item(for: type, at: indexPath.row)  {
                    items.append(item)
                }
            }
        }
        
        return MediaPickerSelection(mediaItems: items)
    }
    
    init(options: MediaPickerOptions) {
        self.options = options
    }
}

extension MediaPickerDataSourceImpl {
    /// Objective-C
    @objc static func dataSource(withOptions optionsDictionary: [String: Bool]) -> MediaPickerDataSourceImpl {
        let options = MediaPickerOptions(dictionary:optionsDictionary)
        let dataSource = MediaPickerDataSourceImpl(options: options)
        return dataSource
    }
    
    @objc func clipMediaItems(with rows: [IndexPath]) -> [ClipMediaItem] {
        return selection(with:rows).mediaItems
    }
}

/// Allow Objective-C to set MediaPickerDataSourceImpl data source
extension MediaPickerViewController {
    @objc var mediaPickerDataSource: MediaPickerDataSourceImpl? {
        set(value) {
            self.dataSource = value
        }
        get {
            if let dataSource = self.dataSource as? MediaPickerDataSourceImpl {
                return dataSource
            }
            return nil
        }
    }    
}
