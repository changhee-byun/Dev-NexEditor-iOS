///**
// * File Name   : ClipSource.swift
// * Description :
// *
// *                NexStreaming Corp. Confidential & Proprietary
// *
// *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
// *                         http://www.nexstreaming.com
// *
// *******************************************************************************
// *     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// *     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// *     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// *     PURPOSE.
// *******************************************************************************
// *
// */
//
//import Foundation
//import UIKit
//import Photos
//import NexEditorFramework
//
//protocol ThumbnailLoadable {
//    func loadThumbnail(size: CGSize,_ loaded: @escaping(_ image: UIImage?)->Void )
//}
//
//protocol ClipSource: ThumbnailLoadable {
//    var nxeClipSource: NXEClipSource { get }
//}
//
//extension PHAsset: ClipSource {
//
//    var nxeClipSource: NXEClipSource {
//        return NXEClipSource(phAsset: self)
//    }
//
//    func loadThumbnail(size: CGSize, _ loaded: @escaping (_ image: UIImage?)->Void )
//    {
//        PHImageManager.default().requestImage(for: self, targetSize: size, contentMode: .aspectFill, options: nil) { (image, info) in
//            loaded(image)
//        }
//    }
//}
//
//extension AVURLAsset: ClipSource {
//    var nxeClipSource: NXEClipSource {
//        return NXEClipSource(avurlAsset: self)
//    }
//
//    func loadThumbnail(size: CGSize,_ loaded: @escaping(_ image: UIImage?)->Void )
//    {
//        DispatchQueue.global().async {
//            let gen = AVAssetImageGenerator(asset: self)
//            gen.appliesPreferredTrackTransform = true
//
//            // Increase image desity as much as the screen scale since thumbnail image is intended for UI
//            let scale = UIScreen.main.scale
//            let scaledSize = CGSize(width: size.width * scale, height: size.height * scale)
//
//            gen.maximumSize = scaledSize
//            let time = CMTime(seconds: 1.0, preferredTimescale: self.duration.timescale)
//            do  {
//                let cgimage = try gen.copyCGImage(at: time, actualTime: nil)
//                let image = UIImage(cgImage: cgimage, scale: scale, orientation: .up)
//                DispatchQueue.main.async {
//                    loaded(image)
//                }
//            } catch (_) {
//                DispatchQueue.main.async {
//                    loaded(nil)
//                }
//            }
//        }
//    }
//}
//
