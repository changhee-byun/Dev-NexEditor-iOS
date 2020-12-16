/*
 * File Name   : AssetCategoriesViewController.swift
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
import UIKit

private let reuseIdentifier = "pickerCell"

/// Lists Asset Categories and allows drill down to asset item groups
class AssetCategoriesViewController: ItemPickerCollectionViewController {
    
    override func viewDidLoad() {
        self.override(reuseId: reuseIdentifier)
        self.dataSource = AssetCategoryPickerDataSource()
        
        super.viewDidLoad()
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    var selectedItem: AssetCategoryItem? {
        var result: AssetCategoryItem? = nil
        if let indexPaths = self.collectionView?.indexPathsForSelectedItems, indexPaths.count > 0 {
            let indexPath = indexPaths[0]
            if let item = self.dataSource?.item(at: indexPath) as? AssetCategoryItem {
                result = item
            }
        }
        return result
    }

    // MARK: - Navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "assetItemGroups" {
            guard let item = self.selectedItem else { return }
            let destination = segue.destination as! AssetItemGroupsViewController
            // destination.
            destination.category = item.category
        }
    }
    
    // MARK: -
    override func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        super.collectionView(collectionView, didSelectItemAt: indexPath)
        self.performSegue(withIdentifier: "assetItemGroups", sender: self)
    }
}

class AssetCategoryItem: ItemPickerItem {
    
    let category: String
    private let imageName: String
    private let displayName: String
    
    lazy var title: String = { return self.displayName  }()
    var desc: String? { return nil }
    
    func loadImage(size: CGSize, loaded: @escaping (UIImage?)->Void) {
        if let image = UIImage(named: imageName)?.resized(size) {
            loaded(image)
        }
    }
    
    init(category: String, displayName: String, imageName: String) {
        self.category = category
        self.displayName = displayName
        self.imageName = imageName
    }
}

extension NXEAssetItemCategory {
 
    // For imageName, prefix 'hero-category-' before loading
    static private let map: [String: (displayName: String, imageName: String)] = [
        NXEAssetItemCategory.effect: (displayName:"Effect", imageName: "effect"),
        NXEAssetItemCategory.transition: (displayName:"Transition", imageName: "transition"),
        NXEAssetItemCategory.template: (displayName:"Template", imageName: "template"),
        NXEAssetItemCategory.textEffect: (displayName:"Text Effect", imageName: "texteffect"),
        NXEAssetItemCategory.collage: (displayName:"Collage", imageName: "collage"),
    ]
    
    static func displayName(for category: String) -> String {
        var result: String = ""
        if let entry = NXEAssetItemCategory.map[category] {
            result = entry.displayName
        }
        return result
    }
    
    static func imageName(for category: String) -> String {
        var result: String = ""
        if let entry = NXEAssetItemCategory.map[category] {
            result = "hero-category-" + entry.imageName
        }
        return result
    }
}

class AssetCategoryPickerDataSource: ItemPickerDataSource {
    
    private let categories = [
        NXEAssetItemCategory.effect,
        NXEAssetItemCategory.transition,
        NXEAssetItemCategory.template,
        NXEAssetItemCategory.textEffect,
        NXEAssetItemCategory.collage
    ]

    var sectionCount: Int {
        return 1
    }
    
    var sectionTitles: [String] = [] // no section header
    
    func loadItems(_ loaded: @escaping ()->Void) {
        loaded()
    }
    
    func itemCount(in section: Int) -> Int {
        return categories.count
    }
    
    func item(at indexPath:IndexPath) -> ItemPickerItem {
        let category = categories[indexPath.row]
        let displayName = NXEAssetItemCategory.displayName(for: category)
        let imageName = NXEAssetItemCategory.imageName(for: category)
        return AssetCategoryItem(category: category, displayName: displayName, imageName:imageName)
    }
}
