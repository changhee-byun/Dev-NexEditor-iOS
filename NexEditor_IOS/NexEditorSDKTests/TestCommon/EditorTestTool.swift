//
//  EditorTestTool.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 5/23/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

enum PlaybackCommand {
    case play(for: TimeInterval)
    case playEnd()
    case pause(for: TimeInterval)
    case resume(for: TimeInterval)
    case seek(to: NXETimeMillis)
    case seekRel(to: NXETimeMillis)
    case seekTo(scale: Float)
}

protocol ClipSequenceProvider {
    var path: String { get }
    var clipNames: [String] { get }
    var name: String { get }
    var ext: String? { get }
    
    func clips() -> [NXEClip]
}

extension ClipSequenceProvider {
    func clips() -> [NXEClip] {
        var clips: [NXEClip] = []
        
        let dirURL = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0].appendingPathComponent(path)
        
        for name in clipNames {
            let resourceName = ext == nil ? name : "\(name).\(ext!)"
            let resourceURL = dirURL.appendingPathComponent(resourceName)
            let clipPath = resourceURL.path
            do {
                let clip = try NXEClip(source: NXEClipSource(path: clipPath))
                clips.append(clip)
            } catch _ {
                XCTFail("Cannot create clip from path: \(clipPath)")
            }
        }
        return clips
    }
}

struct ClipSequence: ClipSequenceProvider {
    let path: String
    let clipNames: [String]
    let name: String
    let ext: String?
    
    init(path: String, clipNames: [String], name: String, ext: String? = nil) {
        self.path = path
        self.clipNames = clipNames
        self.name = name
        self.ext = ext
    }
}

struct TemplateSequenceParams: ClipSequenceProvider {
    
    let sequence : ClipSequence
    var path: String { return sequence.path }
    var clipNames: [String] { return sequence.clipNames }
    var name: String { return sequence.name }
    var ext: String? { return sequence.ext }
    let assetItemIds: [String]

    init(path: String, clipNames: [String], name: String, ext: String? = nil, assetItemIds: [String] = []) {
        self.sequence = ClipSequence(path: path, clipNames: clipNames, name: name, ext: ext)
        self.assetItemIds = assetItemIds
    }
}

extension NXEAspectType {
    
    var displayText: String {
        var result: String
        
        switch self {
        case .ratio16v9: result = "16v9"; break
        case .ratio9v16: result = "9v16"; break
        case .ratio1v1: result = "1v1"; break
        case .ratio1v2: result = "1v2"; break
        case .ratio2v1: result = "2v1"; break
        default: result = "unknown"; break
        }
        return result
    }
    
    func NXESizeIntMake(width: Int, height: Int) -> NXESizeInt {
        var result = NXESizeInt()
        result.width = Int32(width)
        result.height = Int32(height)
        return result
    }
    
    var exportResolution: NXESizeInt {
        var result = NXESizeInt(width: 1920, height: 1080)
        
        switch self {
        case .ratio16v9: result = NXESizeInt(width: 1920, height: 1080); break
        case .ratio9v16: result = NXESizeInt(width: 1080, height: 1920); break
        case .ratio1v1: result = NXESizeInt(width: 1080, height: 1080); break
        case .ratio1v2: result = NXESizeInt(width: 1080, height: 2160); break
        case .ratio2v1: result = NXESizeInt(width: 2160, height: 1080); break
        default: break
        }
        return result
    }
}

extension NXEEngine {
    func play(commands: [PlaybackCommand]) {
        for command in commands {
            switch(command) {
            case .play(let duration):
                self.TTPlay(for: duration)
                break
                
            case .pause(let duration):
                self.pause()
                RunLoop.current.run(until: Date(timeIntervalSinceNow: duration))
                break
                
            case .resume(let duration):
                self.resume()
                RunLoop.current.run(until: Date(timeIntervalSinceNow: duration))
                break
                
            case .seek(let ms):
                self.TTSeek(ms)
                break
                
            case .seekRel(let ms):
                self.TTSeek(ms + self.getCurrentPosition())
                break
                
            case .seekTo(let scale):
                let to = NXETimeMillis(Float(self.project.getTotalTime()) * scale)
                self.TTSeek(to)
                break
                
            case .playEnd():
                self.TTPlayTillEnd()
                break;
            }
        }
    }
}

extension NXEEngine {
    
    func TTPrepare() {
        var prepared = false
        
        preparedEditor {
            print("prepared")
            prepared = true
        }
        
        while !prepared {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }

    func TTPlayTillEnd() {
        var finished = false
        var finishedTime: Date?
        var startTime: Date?
        
        self.setCallbackWithPlayProgress({ (current) in
        }, playStart: {
            startTime = Date()
        }, playEnd: {
            finished = true
            finishedTime = Date()
        })
        self.play()
        
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 2))
        XCTAssert(startTime != nil, "Playback failed starting in 2 seconds")
        
        if startTime == nil {
            return
        }
        
        let timeout = TimeInterval(Double(self.project.getTotalTime() + 1000) / 1000)
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
            
            let progress = Date().timeIntervalSince(startTime!)
            if progress > timeout && !finished {
                // Unfinished playback
                XCTFail("Playback not finished within \(timeout)s")
                break
            }
        }
        
        if finished {
            let finishedS = finishedTime!.timeIntervalSince(startTime!)
            // Finished but 1 second too late
            XCTAssert(finishedS < timeout, "Playback finished too late \(finishedS) >= \(timeout)")
        }
    }
    
    
    func TTPlay(for timeInterval: TimeInterval) {
        XCTAssert(play() == Int32(ERRORCODE.ERROR_NONE.rawValue))
        
        RunLoop.current.run(until: Date(timeIntervalSinceNow: timeInterval))
    }
    
    func TTSeek(_ to: Int32) {
        var finished = false
        setCallbackWithSetTime() { time in
            finished = true
        }
        seek(to)
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
    func TTCaptureFrame(at time: Int32) -> UIImage? {
        var result: UIImage? = nil
        // Seek and Capture
        TTSeek(time)
        
        var finished = false
        captureFrame { image, errorcode in
            result = image
            finished = true
        }
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
        
        return result
    }
    
    func TTPreview(sequence: TemplateSequenceParams, templateId: String, preview: UIView, label: UILabel? = nil, commands: [PlaybackCommand] = [.play(for: 5.0)])
    {
        
        if sequence.clips().count == 0 {
            XCTFail("No clips loaded")
            return
        }
        
        let editor = NXEEngine.instance()!
        let assetLib = NXEAssetLibrary.instance()
        let templateAsset = assetLib.item(forId:templateId)!
        do {
            self.aspectType = templateAsset.aspectType
            
            let project: NXETemplateProject = try NXETemplateProject(templateAssetItem: templateAsset as! NXETemplateAssetItem, clips: sequence.clips())
            editor.setProject(project)
            
            
            // Preview the first frame
            self.setPreviewWith(preview.layer, previewSize: preview.bounds.size)
            self.play(commands: commands)
            self.stopSync { errorcode in }
        } catch {
        }
    }
    
    func TTExport(sequence: TemplateSequenceParams, templateId: String, preview: UIView, label: UILabel? = nil)
    {
        self.TTExport(sequence: sequence, preview: preview, label: label) { ( editor ) -> NXEProject? in
            do {
                let assetLib = NXEAssetLibrary.instance()
                let templateAsset = assetLib.item(forId:templateId)!
                let aspectType = templateAsset.aspectType
                if let label = label {
                    label.text = "Exporting Template \(templateAsset.name!) \(aspectType.displayText) \(sequence.clipNames.count) photos"
                }
                editor.aspectType = aspectType
                let project: NXETemplateProject = try NXETemplateProject(templateAssetItem: templateAsset as! NXETemplateAssetItem, clips: sequence.clips())
                print("mode: \(aspectType.displayText), start template name: \(sequence.name), project clip count: \(project.visualClips.count)")
                return project
            } catch {
                return nil
            }
        }
    }
    
    /// Export routine for convenience
    ///
    /// - Parameters:
    ///   - sequence: clip sequence
    ///   - preview: First a few images of the project will be rendered on this View
    ///   - label: Displayed text on the App's main view controller's label
    ///   - configure: Invoked to allow the caller to configure NXEEngine if required
    func TTExport(sequence: ClipSequenceProvider, preview: UIView? = nil, label: UILabel? = nil, configure: ((NXEEngine)->NXEProject?)? = nil)
    {
        if let label = label {
            label.text = "Exporting \(sequence.name) \(self.aspectType.displayText) \(sequence.clipNames.count) clips"
        }
        var project = configure?(self)
        if (project == nil) {
            project = NXEProject()
            project!.visualClips = sequence.clips()
        }
        self.setProject(project)
        self.TTPrepare()

        // Preview the first frame
        if let preview = preview {
            self.setPreviewWith(preview.layer, previewSize: preview.bounds.size)
            self.play()
            self.pause()
            self.TTSeek(100)
        }
        let resolution = self.aspectType.exportResolution

        // seek to beginning
        TTSeek(0)

        // export
        var finished = false
        var path = NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true)[0]
        path = (path as NSString).appendingPathComponent(sequence.name+"_" + aspectType.displayText + ".mp4")

        self.setCallbackWithEncodingProgress({ (percent) in
            print(percent)
        }, encodingEnd:{ (error) in
            XCTAssert(error == nil, "Export finished with error \(error!.localizedDescription)")
            finished = true
        })

        let exportParams = NXEExportParamsMake(Int32(resolution.width), Int32(resolution.height))
        let ret = self.export(toPath: String(path), with: exportParams)
        XCTAssert(ret == Int32(ERRORCODE.ERROR_NONE.rawValue))

        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
}
