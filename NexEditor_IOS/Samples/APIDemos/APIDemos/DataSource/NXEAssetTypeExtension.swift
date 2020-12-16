/**
 * File Name   : TemplateLoader.swift
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

extension NXEAspectType {
    static let _displayTexts = [
        "16:9",
        "9:16",
        "1:1",
        "1:2",
        "2:1"
    ]
    
    var displayText: String {
        var result: String
        
        let texts = type(of:self)._displayTexts
        let index = Int(self.rawValue)
        if index < texts.count {
            result = texts[index]
        } else {
            result = "unknown"
        }
        return result
    }
}

@objc class NXEAspectTypeExtension: NSObject {

    @objc static func displayTextFor(_ aspectType: NXEAspectType) -> String {
        return aspectType.displayText;
    }
    
    @objc static func displayTexts() -> [String] {
        return NXEAspectType._displayTexts
    }
}
