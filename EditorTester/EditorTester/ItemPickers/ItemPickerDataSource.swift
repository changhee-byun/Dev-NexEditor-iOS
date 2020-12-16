/**
 * File Name   : ItemPickerDataSource.swift
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

protocol ItemPickerItem {
    var title: String {get}
    var desc: String? {get}
    func loadImage(size: CGSize, loaded: @escaping (UIImage?)->Void)
}

struct ReusableCellInfo {
    let reuseId: String
    let cls: AnyClass?
}

protocol ItemPickerViewCell {
    var textLabel: UILabel? { get }
    var imageView: UIImageView? { get }
}

protocol ItemPickerItemView {
    var imageSize: CGSize { get }
    /// cls can be nill if the cell identified by 'reuseId' would be loaded from Storyboard
    var reusableCellClassInfo: ReusableCellInfo? {get}

    func render(_ item: ItemPickerItem, with cell: ItemPickerViewCell)
}



protocol ItemPickerDataSource {
    var sectionCount: Int {get}
    var sectionTitles: [String] {get}
    
    func loadItems(_ loaded: @escaping ()->Void)
    func itemCount(in section: Int) -> Int
    func item(at indexPath:IndexPath) -> ItemPickerItem
    
}
