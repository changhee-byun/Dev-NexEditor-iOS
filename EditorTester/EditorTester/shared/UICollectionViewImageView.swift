/*
 * File Name   : UICollectionViewImageView.swift
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

extension UICollectionViewCell {
    var imageView: UIImageView? {
        return contentView.subviews.filter( {$0.isKind(of: UIImageView.self)} ).first as? UIImageView
    }
    
    func setBorderColor(_ color: UIColor?) {
        if let imageView = self.imageView {
            let borderColor = color?.cgColor ?? nil
            imageView.layer.borderColor = borderColor
            imageView.layer.borderWidth = borderColor != nil ? 1.0 : 0.0
        }
    }
}
