/**
 * File Name   : Types.swift
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

public typealias AspectRatio = NXESizeInt

extension NXESizeInt: Equatable {
    
    static let ratio16v9 = AspectRatio(width:16, height:9)
    static let ratio9v16 = AspectRatio(width:9, height:16)
    
    init() {
        self.init(width: 16, height: 9)
    }
    
    init(h: Int32, v: Int32) {
        self.init(width: h, height: v)
    }

    public static func == (lhs: NXESizeInt, rhs: NXESizeInt) -> Bool {
        return lhs.width == rhs.width && lhs.height == rhs.height
    }
    
    var displayText: String {
        return String(format:"%d x %d", width, height)
    }
    
    var type: NXEAspectType {
        var result: NXEAspectType
        if width == height {
            result = .ratio1v1
        } else if self == type(of:self).ratio16v9 {
            result = .ratio16v9
        } else if self == type(of:self).ratio9v16 {
            result = .ratio9v16
        } else {
            result = .custom
        }
        return result
    }
}
