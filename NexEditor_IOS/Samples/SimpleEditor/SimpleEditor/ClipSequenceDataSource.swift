/**
 * File Name   : ClipSequenceDataSource.swift
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

class ClipSequenceDataSource: NSObject, UICollectionViewDataSource {
    enum Event {
        case didSelect
        case didDeselect
        case didSelectTransition
        case didDeselectTransition
    }
    
    fileprivate var selectedIndex: Int?
    fileprivate var isSelectedTransition = false
    
    let clipCellSize = CGSize(width:90, height:64)
    let transitionCellSize = CGSize(width:20, height:64)

    var clips: [ClipSource] = []
    
    public var on: (Event, ClipSource?, Int?) -> Void = { (_, _, _) in }
    
    init(clips: [ClipSource]) {
        self.clips = clips
        super.init()
    }
    
    func numberOfSections(in collectionView: UICollectionView) -> Int {
        return 1
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        // transitions at even index
        return clips.count + (clips.count - 1)
    }
    let selectedColor = UIColor.yellow.withAlphaComponent(0.7)
    let deselectedColor = UIColor.clear
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let isClip = (indexPath.row % 2) == 0
        let clipIndex = indexPath.row / 2
        let reuseId = isClip ? "clipCell" : "clipTransitionCell"
        
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: reuseId, for: indexPath)
        let imageView = cell.contentView.subviews.filter( {$0.isKind(of: UIImageView.self)} )[0] as! UIImageView
        
        if isClip {
            let clip = clips[clipIndex]
            let size = collectionView.collectionViewLayout.layoutAttributesForItem(at: indexPath)!.size
            clip.loadThumbnail(size: size) { image in
                imageView.image = image
            }
        }
        
        if cell.isSelected {
            cell.backgroundColor = selectedColor
        } else {
            cell.backgroundColor = deselectedColor
        }
        
        return cell
    }
}

extension ClipSequenceDataSource: UICollectionViewDelegateFlowLayout {
    
    var selectedClip: ClipSource? {
        var result: ClipSource? = nil
        if let index = self.selectedIndex {
            result = clips[index]
        }
        return result
    }
    
    func collectionView(_ collectionView: UICollectionView,
                        layout collectionViewLayout: UICollectionViewLayout,
                        sizeForItemAt indexPath: IndexPath) -> CGSize {

        let isClip = (indexPath.row % 2) == 0
        if isClip {
            return clipCellSize
        } else {
            return transitionCellSize
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        if let cell = collectionView.cellForItem(at: indexPath) {
            cell.backgroundColor = selectedColor
            let isClip = (indexPath.row % 2) == 0
            let clipIndex = indexPath.row / 2
            
            selectedIndex = clipIndex
            isSelectedTransition = !isClip
            
            let event: Event = isClip ? .didSelect : .didSelectTransition
            on(event, clips[clipIndex], clipIndex)
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, didDeselectItemAt indexPath: IndexPath) {
        if let cell = collectionView.cellForItem(at: indexPath) {
            cell.backgroundColor = deselectedColor
            let isClip = (indexPath.row % 2) == 0
            let clipIndex = indexPath.row / 2

            selectedIndex = nil
            
            let event: Event = isClip ? .didDeselect : .didDeselectTransition
            on(event, clips[clipIndex], clipIndex)
        }
    }
}
