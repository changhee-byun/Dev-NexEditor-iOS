/**
 * File Name   : ThumbnailSequenceDataSource.swift
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
import UIKit
import Photos

private let navyBlue = UIColor(red: 0, green: 0, blue: 0x80, alpha: 1)

class ThumbnailSequenceDataSource: NSObject, UICollectionViewDataSource {
    private let cellReuseId: String
    let selectedColor = navyBlue
    var items: [ThumbnailLoadable] = []
    
    init(items: [ThumbnailLoadable], cellReuseId: String) {
        self.cellReuseId = cellReuseId
        self.items = items
        super.init()
    }
    
    func numberOfSections(in collectionView: UICollectionView) -> Int {
        return 1
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        // transitions at even index
        return items.count
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: cellReuseId, for: indexPath)
        let imageView = cell.imageView!
        imageView.layer.cornerRadius = 8
        imageView.contentMode = .scaleAspectFill
        imageView.image = nil
        let item = items[indexPath.row]
        let size = collectionView.collectionViewLayout.layoutAttributesForItem(at: indexPath)!.size
        item.loadThumbnail(size: size) { image in
            imageView.image = image
        }
        
        cell.setBorderColor(cell.isSelected ? selectedColor : nil )
        
        return cell
    }
}

class ThumbnailSequenceCollectionViewDelegate: NSObject, UICollectionViewDelegate {
    let dataSource: ThumbnailSequenceDataSource
    public var onSelection: ((_ ds: ThumbnailSequenceCollectionViewDelegate, _ item: ThumbnailLoadable?, _ index: Int?)->Void)?

    init(dataSource: ThumbnailSequenceDataSource) {
        self.dataSource = dataSource
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        if let cell = collectionView.cellForItem(at: indexPath) {
            cell.setBorderColor(dataSource.selectedColor)
            onSelection?(self, dataSource.items[indexPath.row], indexPath.row)
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, didDeselectItemAt indexPath: IndexPath) {
        if let cell = collectionView.cellForItem(at: indexPath) {
            cell.setBorderColor(nil)
            onSelection?(self, nil, NSNotFound)
        }
        
    }
}
