/**
 * File Name   : ExportSettingDataSourceImpl.swift
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

extension ExportSetting: ExportSettingItem {
    var setting: ExportSetting { return self }
    var title: String { return displayText }
}

@objc class ExportSetting: NSObject {
    
    enum Keys: String {
        case width      = "width"
        case height     = "height"
        case bitrate    = "bitrate"
        case aspectType = "aspectType"
        case fps        = "fps"
    }
    
    @objc let width: Int
    @objc let height: Int
    @objc let aspectType: NXEAspectType
    @objc let bitrate: Int
    @objc let fps: Int
    
    init(width: Int, height: Int, aspectType: NXEAspectType, bitrate: Int, fps: Int) {
        self.width = width
        self.height = height
        self.aspectType = aspectType
        self.bitrate = bitrate
        self.fps = fps
        super.init()
    }
    
    convenience init(_ width: Int, _ height: Int, _ aspectType: NXEAspectType, _ bitrate: Int, _ fps: Int) {
        self.init(width: width, height: height, aspectType: aspectType, bitrate: bitrate, fps: fps)
    }
    
    convenience init(dictionary: [String: Int]) {
        let width = dictionary[Keys.width.rawValue] ?? 1920
        let height = dictionary[Keys.height.rawValue] ?? 1080
        let bitrate = dictionary[Keys.bitrate.rawValue] ?? 11520000
        let fps = dictionary[Keys.fps.rawValue] ?? 30
        
        let aspectType: NXEAspectType
        if let rawType = dictionary[Keys.aspectType.rawValue] {
            aspectType = NXEAspectType(rawValue: UInt(rawType)) ?? .ratio16v9
        } else {
            aspectType = .ratio16v9
        }
        
        self.init(width: width, height: height, aspectType: aspectType, bitrate: bitrate, fps: fps)
    }
    
    func dictionary() -> [String: Int] {
        return [Keys.aspectType.rawValue: Int(aspectType.rawValue),
                Keys.width.rawValue: width,
                Keys.height.rawValue: height,
                Keys.bitrate.rawValue: bitrate,
                Keys.fps.rawValue: fps]
    }
    
    @objc var displayText: String {
        return String(format: "%4d x %4d @%d fps - %@",  width, height, fps, StringTools.humanReadableBits(bitrate))
    }
    
}

@objc class ExportSettings: NSObject {
    @objc static let userDefaultKey = "exportSettings"
    
    private static let defaultSettings: [ExportSetting] = [
        //16v9
        ExportSetting(3840, 2160, .ratio16v9, 11520000, 30),
        ExportSetting(1920, 1080, .ratio16v9, 11520000, 30),
        ExportSetting(1280, 720,  .ratio16v9,  4608000, 30),
        ExportSetting( 640, 360,  .ratio16v9,  1024000, 30),
        //9v16
        ExportSetting(1080, 1920, .ratio9v16, 11520000, 30),
        ExportSetting( 720, 1280, .ratio9v16,  4608000, 30),
        ExportSetting( 360,  640, .ratio9v16,  1024000, 30),
        //1v1
        ExportSetting( 360,  360, .ratio1v1,    645468, 30),
        ExportSetting( 720,  720, .ratio1v1,   2581875, 30),
        ExportSetting(1080, 1080, .ratio1v1,   5809219, 30),
        //1v2
        ExportSetting( 360,  720, .ratio1v2,   1290937, 30),
        ExportSetting( 720, 1440, .ratio1v2,   5163750, 30),
        ExportSetting(1080, 2160, .ratio1v2,  11618438, 30),
        //2v1
        ExportSetting( 720,  360, .ratio2v1,   1290937, 30),
        ExportSetting(1440,  720, .ratio2v1,   5163750, 30),
        ExportSetting(2160, 1080, .ratio2v1,  11618438, 30),
    ]

    @objc static func dictionariesForDefaultSettings() -> [[String: Int]] {
        var settings: [[String : Int]] = []
        for entry in defaultSettings {
            settings.append(entry.dictionary())
        }
        return settings
    }
    @objc static func settings() -> [ExportSetting] {
        var dictionaries = UserDefaults.standard.object(forKey: userDefaultKey) as? [[String : Int]]
        if dictionaries == nil {
            dictionaries = dictionariesForDefaultSettings()
            UserDefaults.standard.setValue(dictionaries, forKey: userDefaultKey)
        }
        let entries = dictionaries!
        return entries.map({ ExportSetting(dictionary:$0) }).sorted(by: {($0.width * $0.height) > ($1.width * $1.height) })
    }
    
    @objc static func settings(with aspectType: NXEAspectType) -> [ExportSetting] {
        return settings().filter({ $0.aspectType == aspectType })
    }
    
    @objc static func resetToDefault() {
        UserDefaults.standard.setValue(nil, forKey: userDefaultKey)
    }
    
    @objc static func saveSettings(_ settings: [ExportSetting]) {
        let dictionaries = settings.map( { $0.dictionary() })
        UserDefaults.standard.setValue(dictionaries, forKey: ExportSettings.userDefaultKey)
    }
}

extension ExportSetting {
    convenience init(exportParams params: NXEExportParams, aspectType: NXEAspectType) {
        self.init(Int(params.width), Int(params.height), aspectType, Int(params.bitPerSec), Int(params.framePerSec))
    }
}

class ExportSettingDataSourceImpl : NSObject, ExportSettingDataSource
{
    var exportItems = [ExportSetting]()
    
    func loadItems(_ loaded: @escaping ()->Void) {
        self.exportItems = ExportSettings.settings()
        loaded()
    }
    
    func reset( completion: @escaping () -> Void ) {
        ExportSettings.resetToDefault()
        self.exportItems = ExportSettings.settings()
        self.loadItems {
            completion()
        }
    }
    
    func itemCount(in section: Int) -> Int
    {
        return self.exportItems.count
    }
    
    func item(at indexPath:IndexPath) -> ExportSettingItem
    {
        return self.exportItems[indexPath.row]
    }
    
    func updateItem(at indexPath: IndexPath, with setting:ExportSetting, completion: @escaping () -> Void )
    {
        self.exportItems.remove(at: indexPath.row)
        self.exportItems.insert(setting, at: indexPath.row)
        
        ExportSettings.saveSettings( self.exportItems )
        completion()
    }
}
