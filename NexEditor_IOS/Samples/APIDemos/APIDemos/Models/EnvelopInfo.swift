/*
 * File Name   : EnvelopInfo.swift
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

import Foundation

class EnvelopInfo: NSObject {
    @objc let time: Int
    @objc let level: Int
    
    override var description: String {
        return StringTools.formatTimeMs(time) + ",\(level)"
    }
    
    @objc init(time: Int, level: Int) {
        self.time = time
        self.level = level
        super.init()
    }
    
    // MARK: - More suitable as [EnvelopInfo] methods
    @objc static func descriptionOf(list: [EnvelopInfo]) -> String {
        return list.map{ $0.description }.joined(separator: "\n")
    }
    
    @objc static func indexOf(infoWithTime time: Int, in list: [EnvelopInfo]) -> Int {
        var found: Int = NSNotFound
        for (index, info) in list.enumerated() {
            if info.time == time {
                found = index
                break;
            }
        }
        return found
    }
    
    @objc static func sortListByTime(_ list: [EnvelopInfo]) -> [EnvelopInfo] {
        return list.sorted(by: { $0.time < $1.time })
    }
    
    @objc static func defaultListWith(duration: Int) -> [EnvelopInfo] {
        return [
            EnvelopInfo(time: 0, level: 100),
            EnvelopInfo(time: duration, level: 100)
        ]
    }
    
    /// Add or replace info in the list with the new one.
    @objc static func mergeList(_ list: [EnvelopInfo], with info: EnvelopInfo) -> [EnvelopInfo] {
        var result = list
        let index = indexOf(infoWithTime: info.time, in: list)
        if index != NSNotFound {
            result[index] = info
        } else {
            result.append(info)
            result = sortListByTime(result)
        }
        return result
    }
}

