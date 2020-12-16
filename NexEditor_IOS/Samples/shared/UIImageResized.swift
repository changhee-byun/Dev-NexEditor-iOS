/**
 * File Name   : UIImageResized.swift
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
