/**
 * File Name   : ClipMediaItemExtensions.swift
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

/// Add loadThumbnail() method loads thumbnail if the string is path to a proper image file
extension String: ThumbnailLoadable {
    func loadThumbnail(size: CGSize,_ loaded: @escaping(_ image: UIImage?)->Void ) {
        DispatchQueue.main.async {
            if let image = UIImage(contentsOfFile: self) {
                let resized = image.resized(size) ?? image
                loaded(resized)
            } else {
                loaded(nil)
            }
        }
    }
}

/// Add AVURLAsset property to PHAsset
extension PHAsset {
    
    var AVURLAsset: AVURLAsset? {
        
        if self.mediaType != .video {
            return nil
        }
        
        var result: AVURLAsset? = nil
        let dg = DispatchGroup()
        dg.enter()
        PHImageManager.default().requestAVAsset(forVideo: self, options: nil, resultHandler: { (avasset, audioMix, info) in
            if let urlAsset = avasset as? AVURLAsset {
                result = urlAsset
            }
            dg.leave()
        })
        dg.wait()
        
        return result
    }
}

extension ClipMediaType {
    var displayText: String {
        switch self {
        case .video: return "Video"
        case .image: return "Image"
        case .audio: return "Audio"
        }
    }
}

extension ClipMediaItem {
    var clipSource: NXEClipSource? {
        var result: NXEClipSource? = nil
        if self.phAsset != nil {
            result = NXEClipSource(phAsset: self.phAsset!)
        } else if self.filePath != nil {
            result = NXEClipSource(path: self.filePath!)
        }
        return result
    }
    
    /// Creates and returns AVURLAsset of the media if it's a video
    var avurlAsset: AVURLAsset? {
        var result: AVURLAsset? = nil
        
        if self.type == .video {
            if let phAsset = self.phAsset {
                result = phAsset.AVURLAsset
            } else if let path = self.filePath {
                result = AVURLAsset(url: URL(fileURLWithPath: path))
            }
        }
        return result
    }
    
    func loadThumbnail(size: CGSize, _ loaded: @escaping(_ image: UIImage?)->Void )
    {
        let loadable : ThumbnailLoadable?
        if let phAsset = self.phAsset {
            loadable = phAsset
        } else if let avurlAsset = self.avurlAsset {
            loadable = avurlAsset
        } else if let path = self.filePath {
            loadable = path
        } else {
            loadable = nil
        }
        
        if let loadable = loadable {
            loadable.loadThumbnail(size: size, loaded)
        } else {
            loaded(nil)
        }
    }
}
