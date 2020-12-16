/**
 * File Name   : AssetCollectionViewDataSource.swift
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
import NexEditorFramework

/// Cell Reuse identifier required: "assetCell"
class AssetCollectionViewDataSource : NSObject, UICollectionViewDataSource {
    
    var assets: [NXEAssetItem] = []
    init(category: String) {
        
        let lib = NXEAssetLibrary.instance()
        assets = lib.items(inCategory: category)
        super.init()
    }
    
    func numberOfSections(in collectionView: UICollectionView) -> Int {
        return 1
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return assets.count
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "assetCell", for: indexPath)
        let imageView = cell.contentView.subviews.filter( {$0.isKind(of: UIImageView.self)} )[0] as! UIImageView
        let label = cell.contentView.subviews.filter( {$0.isKind(of:UILabel.self)})[0] as! UILabel
        
        
        let item = assets[indexPath.row]
        label.text = item.label
        if item.hasIcon {
            item.loadIconImageData({ (imageData, ext) in
                if imageData != nil && (ext == "png" || ext == "jpg") {
                    DispatchQueue.main.async {
                        imageView.image = UIImage(data: imageData!)
                    }
                }
            })
        }
        
        if cell.isSelected {
            cell.backgroundColor = UIColor.blue.withAlphaComponent(0.7)
        } else {
            cell.backgroundColor = UIColor.darkGray
        }
        
        return cell
    }
    
    func item(at indexPath: IndexPath) -> NXEAssetItem? {
        if indexPath.row < assets.count {
            return assets[indexPath.row]
        }
        return nil
    }
    
}
