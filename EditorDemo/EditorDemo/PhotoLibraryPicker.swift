//
//  VideoPicker.swift
//  TranscodingTest
//
//  Created by CHANGHEE2 on 2020/12/08.
//

import SwiftUI
import PhotosUI

typealias PhotoLibraryPickerCompletion = (_ asset: Any, _ fileName: String, _ thumbnail: UIImage, _ fetchResult: PHFetchResult<PHAsset>) -> Void

struct PhotoLibraryPicker: UIViewControllerRepresentable {
    let configuration: PHPickerConfiguration
    //@Binding var pickerResult: [Any]
    @Binding var isPresented: Bool
    var onPicked: PhotoLibraryPickerCompletion
    
    func makeUIViewController(context: Context) -> PHPickerViewController {
        let controller = PHPickerViewController(configuration: configuration)
        controller.delegate = context.coordinator
        return controller
    }
    
    func updateUIViewController(_ uiViewController: PHPickerViewController, context: Context) { }
    
    func makeCoordinator() -> Coordinator {
        Coordinator(self)
    }
    
    /// PHPickerViewControllerDelegate => Coordinator
    class Coordinator: PHPickerViewControllerDelegate {
        
        private let parent: PhotoLibraryPicker
        var pickerResult: [Any] = []
        
        init(_ parent: PhotoLibraryPicker) {
            self.parent = parent
        }
        
        func getAssetInfo(pickerResults: [PHPickerResult], index: Int, completion: @escaping ((_ name: String, _ responseURL : URL?, _ thumbnail: UIImage) -> Void))-> Void {
            let identifiers = pickerResults.compactMap(\.assetIdentifier)
            let fetchResult = PHAsset.fetchAssets(withLocalIdentifiers: identifiers, options: nil)
            let firstAsset = fetchResult[index]
            let fileName = firstAsset.value(forKey: "filename") as? String
            print("asset name -> \(String(describing: fileName))")
            
            let manager = PHImageManager.default()
            let option = PHImageRequestOptions()
            var thumbnail = UIImage()
            option.isSynchronous = true
            manager.requestImage(for: firstAsset, targetSize: CGSize(width: 138, height: 138), contentMode: .aspectFit, options: option, resultHandler: {(result, info)->Void in
                thumbnail = result!
                firstAsset.getURL() { url  in
                    print("asset url -> \(url)")
                    completion(fileName ?? "noname", url, thumbnail)
                }
            })
            
            
        }
        
        func picker(_ picker: PHPickerViewController, didFinishPicking results: [PHPickerResult]) {
            for (index, asset) in results.enumerated() {
                if asset.itemProvider.canLoadObject(ofClass: UIImage.self)  {
                    asset.itemProvider.loadObject(ofClass: UIImage.self) { (newImage, error) in
                        if let error = error {
                            print(error.localizedDescription)
                        } else {
                            //self.parent.pickerResult.append(newImage as Any)
                            self.pickerResult.append(newImage as Any)
                            self.getAssetInfo(pickerResults: results, index: index) { fileName, url, thumbnail in
                                //guard let url = url else { return }
                                let identifiers = results.compactMap(\.assetIdentifier)
                                let fetchResult = PHAsset.fetchAssets(withLocalIdentifiers: identifiers, options: nil)
                                self.parent.onPicked(newImage as Any, fileName, thumbnail, fetchResult)
                            }
                        }
                    }
                }
                else if let itemProvider = results.first?.itemProvider, itemProvider.hasItemConformingToTypeIdentifier("public.movie") {
                    DispatchQueue.main.async {
                        self.getAssetInfo(pickerResults: results, index: index) { fileName, url, thumbnail in
                            guard let url = url else { return }
                            let identifiers = results.compactMap(\.assetIdentifier)
                            let fetchResult = PHAsset.fetchAssets(withLocalIdentifiers: identifiers, options: nil)
                            self.parent.onPicked(url, fileName, thumbnail, fetchResult)
                        }
                    }
                }
                else {
                    NSLog("Not supported asset")
                }
            }
            parent.isPresented = false
        }
    }
}

extension PHAsset {
    func getURL(completionHandler : @escaping ((_ responseURL : URL?) -> Void)){
        if self.mediaType == .image {
            let options: PHContentEditingInputRequestOptions = PHContentEditingInputRequestOptions()
            options.canHandleAdjustmentData = {(adjustmeta: PHAdjustmentData) -> Bool in
                return true
            }
            self.requestContentEditingInput(with: options, completionHandler: {(contentEditingInput: PHContentEditingInput?, info: [AnyHashable : Any]) -> Void in
                completionHandler(contentEditingInput!.fullSizeImageURL as URL?)
            })
        } else if self.mediaType == .video {
            let options: PHVideoRequestOptions = PHVideoRequestOptions()
            options.version = .original
            PHImageManager.default().requestAVAsset(forVideo: self, options: options, resultHandler: {(asset: AVAsset?, audioMix: AVAudioMix?, info: [AnyHashable : Any]?) -> Void in
                if let urlAsset = asset as? AVURLAsset {
                    let localVideoUrl: URL = urlAsset.url as URL
                    completionHandler(localVideoUrl)
                } else {
                    completionHandler(nil)
                }
            })
        }
    }
}

