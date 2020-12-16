/*
 * File Name   : AssetPickerDataSource.swift
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

protocol ImageLoadable {
    func loadImage(size: CGSize, loaded: @escaping (UIImage?)->Void)
}

extension NXEAssetItem: ImageLoadable {
    
    func loadImage(size: CGSize, loaded: @escaping (UIImage?)->Void) {
        if self.hasIcon {
            self.loadIconImageData({ (data, ext) in
                if let data = data, let image = UIImage(data: data) {
                    loaded(image)
                }
            })
        }
    }
}

class AssetPickerItem: ItemPickerItem {
    
    let assetItem: NXEAssetItem
    
    lazy var title: String = {
        return self.assetItem.name ?? ""
    }()
    
    lazy var desc: String? = {
        return String(format:"%@ %@", self.assetItem.aspectType.displayText, self.assetItem.desc)
    }()
    
    func loadImage(size: CGSize, loaded: @escaping (UIImage?)->Void) {
        assetItem.loadImage(size: size) { image in
            if let image = image?.resized(size) {
                loaded(image)
            }
        }
    }
    
    init(_ assetItem: NXEAssetItem) {
        self.assetItem = assetItem
    }
}

class AssetPickerDataSource: ItemPickerDataSource {
    
    let aspectTypes: [NXEAspectType]
    let category: String
    
    private var itemsPerMode: [[NXEAssetItem]] = []
    
    var sectionCount: Int {
        if itemsPerMode.count == 0 {
            // dummy entries until loaded
            for _ in 0..<aspectTypes.count {
                itemsPerMode.append([])
            }
        }
        return aspectTypes.count
    }
    
    lazy var sectionTitles: [String] = {
        return self.aspectTypes.map { $0.displayText }
    }()
    
    func loadItems(_ loaded: @escaping ()->Void) {
        let lib = NXEAssetLibrary.instance()
        let aspectTypes = self.aspectTypes
        
        lib.items(inCategory: category) { [weak self] (items) in
            var perMode: [[NXEAssetItem]] = []
            for type in aspectTypes {
                perMode.append(items.filter { $0.aspectType == type })
            }
            self?.itemsPerMode = perMode
            loaded()
        }
    }
    
    func itemCount(in section: Int) -> Int {
        return itemsPerMode[section].count
    }
    
    func item(at indexPath:IndexPath) -> ItemPickerItem {
        let assetItem = itemsPerMode[indexPath.section][indexPath.row]
        return AssetPickerItem(assetItem)
    }
    
    init(category: String, aspectTypes: [NXEAspectType]) {
        var types: [NXEAspectType] = aspectTypes
        if types.count == 0 {
            types = [.ratio16v9, .ratio9v16, .ratio1v1, .ratio2v1, .ratio1v2]
        }
        self.aspectTypes = types
        self.category = category
    }
}

class TemplatePickerDataSource: AssetPickerDataSource {
    public static let allAspectTypes: [NXEAspectType] = [.ratio16v9, .ratio9v16, .ratio1v1, .ratio2v1]
    
    convenience init() {
        self.init(aspectTypes: type(of:self).allAspectTypes)
    }
    
    init(aspectTypes: [NXEAspectType]) {
        super.init(category: NXEAssetItemCategory.template,
                   aspectTypes: aspectTypes)
    }
}

class TextEffectPickerDataSource: AssetPickerDataSource {
    init(_ aspectType: NXEAspectType) {
        super.init(category: NXEAssetItemCategory.textEffect,
                   aspectTypes: [aspectType])
    }
}
