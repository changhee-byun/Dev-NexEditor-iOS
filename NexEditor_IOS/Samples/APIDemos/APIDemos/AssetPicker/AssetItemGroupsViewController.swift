/*
 * File Name   : AssetItemGroupsViewController.swift
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
 */

import Foundation

private let reuseIdentifier = "pickerCell"

/// Lists asset item groups and selecting a group lists asset items with different attributes in the group
class AssetItemGroupsViewController: ItemPickerCollectionViewController {
    var category: String = "template"   // non-null, fallback default is 'template'
    
    override func viewDidLoad() {
        self.override(reuseId: reuseIdentifier)
        self.title = NXEAssetItemCategory.displayName(for: self.category)
        self.dataSource = AssetItemGroupDataSource(category: self.category)

        super.viewDidLoad()
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    override func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        super.collectionView(collectionView, didSelectItemAt: indexPath)
        
        let item = self.dataSource?.item(at: indexPath) as! AssetItemGroupPickerItem
        let destination = self.storyboard?.instantiateViewController(withIdentifier: "assetItemPicker") as! ItemPickerCollectionViewController
        destination.title = item.group.localizedName
        destination.override(reuseId: "pickerCell")
        destination.dataSource = AssetInGroupItemsDataSource(group: item.group)
        self.navigationController?.pushViewController(destination, animated: true)
    }
}

extension NXEAssetItem {
    var attributeDisplayText: String {
        var result: String = ""
        let category = self.category
        if category == NXEAssetItemCategory.collage {
            // "16:9 3 slots"
            let collageItem = self as! NXECollageAssetItem
            if let info = collageItem.info {
                result = "\(info.aspectRatio.width):\(info.aspectRatio.height) \(info.slots.count) slots"
            }
        } else if category == NXEAssetItemCategory.template || category == NXEAssetItemCategory.textEffect {
            // "16:9"
            result = self.aspectType.displayText
        } else {
            //
            result = self.name ?? ""
        }
        return result
    }
}

class AssetInGroupItem: ItemPickerItem {
    
    let assetItem: NXEAssetItem
    
    lazy var title: String = {
        return self.assetItem.attributeDisplayText
    }()
    
    var desc: String? = nil
    
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

class AssetInGroupItemsDataSource: ItemPickerDataSource {
    
    let group: NXEAssetItemGroup
    
    var sectionCount: Int = 1
    var sectionTitles: [String] = []
    
    func loadItems(_ loaded: @escaping ()->Void) {
        loaded()
    }
    
    func itemCount(in section: Int) -> Int {
        return self.group.items.count
    }
    
    func item(at indexPath:IndexPath) -> ItemPickerItem {
        let assetItem = group.items[indexPath.row]
        return AssetInGroupItem(assetItem)
    }
    
    init(group: NXEAssetItemGroup) {
        self.group = group
    }
}

class AssetItemGroupPickerItem: ItemPickerItem {
    
    let group: NXEAssetItemGroup
    
    var title: String {  return group.localizedName }
    
    var desc: String? = nil
    
    func loadImage(size: CGSize, loaded: @escaping (UIImage?) -> Void) {
        group.loadIconImageData { (data, ext) in
            var image: UIImage? = nil
            if data != nil && (ext == "png" || ext == "jpg") {
                image = UIImage(data: data!)?.resized(size)
            }
            loaded(image)
        }
    }
    
    init(group: NXEAssetItemGroup) {
        self.group = group
    }
    
}

class AssetItemGroupDataSource: ItemPickerDataSource {
    let category: String
    var groups: [NXEAssetItemGroup] = []
    
    var sectionCount: Int = 1
    var sectionTitles: [String] = []    // no section header
    
    func loadItems(_ loaded: @escaping () -> Void) {
        let lib = NXEAssetLibrary.instance()
        let category = self.category
        
        DispatchQueue.global().async { [weak self] in
            self?.groups = lib.groups(inCategory: category)
            DispatchQueue.main.async {
                loaded()
            }
        }
    }
    
    func itemCount(in section: Int) -> Int {
        return groups.count
    }
    
    func item(at indexPath: IndexPath) -> ItemPickerItem {
        let group = groups[indexPath.row]
        return AssetItemGroupPickerItem(group: group)
    }
    
    init(category: String) {
        self.category = category
    }
}
