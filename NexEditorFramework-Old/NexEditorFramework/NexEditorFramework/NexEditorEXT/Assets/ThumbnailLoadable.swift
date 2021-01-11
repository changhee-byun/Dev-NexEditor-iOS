//
//  ThumbnailLoadable.swift
//  NexEditorFramework
//
//  Created by CHANGHEE2 on 2021/01/08.
//  Copyright © 2021 NexStreaming Corp. All rights reserved.
//

import Foundation


public protocol ThumbnailLoadable {
    func loadThumbnail(size: CGSize,_ loaded: @escaping(_ image: UIImage?)->Void )
}

extension NXEAssetItemGroup: ThumbnailLoadable {
    public func loadThumbnail(size: CGSize,_ loaded: @escaping(_ image: UIImage?)->Void ) {
        loadIconImageData { (data, ext) in
            var image: UIImage? = nil
            if data != nil && (ext == "png" || ext == "jpg") {
                image = UIImage(data: data!)?.resized(size)
            }
            loaded(image)
        }
    }
}

extension NXEAssetItem: ThumbnailLoadable {
    public func loadThumbnail(size: CGSize,_ loaded: @escaping(_ image: UIImage?)->Void ) {
        loadIconImageData { (data, ext) in
            var image: UIImage? = nil
            if data != nil && (ext == "png" || ext == "jpg") {
                image = UIImage(data: data!)?.resized(size)
            }
            loaded(image)
        }
    }
}

extension UIImage {
    func resized(_ size: CGSize) -> UIImage? {
        
        if size.width >= self.size.width || size.height >= self.size.height {
            return self
        }
        
        let srcSize = self.size
        let scalex = size.width / srcSize.width
        let scaley = size.height / srcSize.height
        let scale = scalex > scaley ? scalex : scaley
        
        let scaledSize = CGSize(width: srcSize.width * scale, height: srcSize.height * scale)
        UIGraphicsBeginImageContextWithOptions((scaledSize), false, self.scale)
        self.draw(in: CGRect(x: 0, y: 0, width: scaledSize.width, height: scaledSize.height))
        let result = UIGraphicsGetImageFromCurrentImageContext()
        UIGraphicsEndImageContext()
        return result
    }
}


extension UIImage: ThumbnailLoadable {
    public func loadThumbnail(size: CGSize, _ loaded: @escaping (UIImage?) -> Void) {
        loaded(self.resized(size))
    }
}

extension NXEAssetItemGroup {
    func itemWith(slotCount count: Int, nearest: Bool) -> NXECollageAssetItem? {
        if items.count == 0 { return nil }
        
        var result: NXECollageAssetItem? = nil
        
        let filtered = items.filter({ (($0 as? NXECollageAssetItem)?.info.slots.count ?? 0) == count})
        result = filtered.first as? NXECollageAssetItem
        
        if result == nil && nearest {
            let sorted = (items as! [NXECollageAssetItem]).sorted(by: { $0.info.slots.count < $1.info.slots.count})
            result = (count == 0) ? sorted.first : sorted.last
        }
        return result
    }
}
