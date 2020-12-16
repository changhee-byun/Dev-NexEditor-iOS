//
//  PhotoPickerView.swift
//  EditorTester
//
//  Created by ERIC0 on 2020/11/02.
//

import SwiftUI
import PhotosUI

struct PhotoPickerView: UIViewControllerRepresentable {
    let configuration: PHPickerConfiguration
    //@Binding var pickerResult: [UIImage]
    @Binding var pickerResult: [NXEClipSource?]
    @Binding var isPresented: Bool
    
    func makeUIViewController(context: Context) -> PHPickerViewController {
        let controller = PHPickerViewController(configuration: configuration)
        controller.delegate = context.coordinator
        return controller
    }
    func updateUIViewController(_ uiViewController: PHPickerViewController, context: Context) { }
    func makeCoordinator() -> Coordinator {
        Coordinator(self)
        
    }
    
    class Coordinator: PHPickerViewControllerDelegate {
        private let parent: PhotoPickerView
        init(_ parent: PhotoPickerView) {
            self.parent = parent
        }
        func picker(_ picker: PHPickerViewController, didFinishPicking results: [PHPickerResult]) {
            let identifiers = results.compactMap(\.assetIdentifier)
            let fetchResult = PHAsset.fetchAssets(withLocalIdentifiers: identifiers, options: nil)
            for index in 0..<fetchResult.count {
                self.parent.pickerResult.append(NXEClipSource(phAsset: fetchResult[index]))
            }
            
            
            parent.isPresented = false
        }
    }
}

struct PhotoPickerView_Previews: PreviewProvider {

    static var previews: some View {
        Text("asdf")
//        PhotoPickerView(configuration:self.config,
//                        pickerResult:$pickerResult,
//                        isPresented: $isPresented)
    }
}
