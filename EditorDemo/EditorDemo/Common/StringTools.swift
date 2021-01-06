/**
 * File Name   : StringTools.swift
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

class StringTools: NSObject {
    /// Formats number into K/M/Gbps like notation
    /// For example, humanReadableBits(1000 * 1000, suffix: "bytes", unit: 1000) -> "1Mbytes"
    @objc static func humanReadableBits(_ bits: Int, suffix: String? = "bps", unit: Int = 1000 ) -> String {
        let pres: [String] = [ "K", "M", "G", "T", "P", "E" ]
        
        let value: Double
        let prefix: String?
        if (bits >= unit) {
            let exp = Int(log(Double(bits)) / log (Double(unit)))
            prefix = pres[exp-1]
            value = Double(bits) / pow(Double(unit), Double(exp))
        } else {
            prefix = nil
            value = Double(bits)
        }
        
        return String(format: "%.1f \(prefix ?? "")\(suffix ?? "")", value)
    }
    
    @objc static func timeFormatter(dateFormat: String = "HH:mm:ss.SS") -> DateFormatter{
        let formatter = DateFormatter()
        formatter.timeZone = TimeZone(secondsFromGMT: 0)
        formatter.dateFormat = dateFormat
        return formatter
    }
    
    /// 'HH:mm:ss' formatter
    /// ```
    /// let formatter = durationFormatter
    /// let string = formatter.string(from: Date(timeIntervalSinceReferenceDate: 300))
    /// -> '00:05:00'
    /// ```
    @objc static var durationFormatter: DateFormatter = {
        return timeFormatter(dateFormat: "mm:ss.S")
    }()
    
    @objc static var hourDurationFormatter: DateFormatter = {
        return timeFormatter(dateFormat: "HH:mm:ss.S")
    }()
    
    /// formatTime(milliseconds: 300) -> '00:05:00'
    @objc static func formatTimeMs(_ milliseconds: Int) -> String {
        let interval = TimeInterval(Double(milliseconds) / 1000.0)
        return formatTime(interval: interval)
    }
    
    @objc static func formatTime(interval: TimeInterval) -> String {
        let hour = 3600.0
        let time = Date(timeIntervalSinceReferenceDate: interval)
        
        let formatter: DateFormatter
        if interval > hour {
            formatter = hourDurationFormatter
        } else {
            formatter = durationFormatter
        }
        return formatter.string(from: time)
    }
}
