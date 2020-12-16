/**
 * File Name   : ProjectTests.swift
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

import XCTest
@testable import NexEditorFramework

extension XCTestCase {
    
    /// Use this method only if project/clip creations are out of test scope
    func typicalProject(clipSubPaths subpaths: [String]) throws -> NXEProject {
        let project = NXEProject()
        
        let paths = subpaths.map({ clipsourceUrl.appendingPathComponent($0).path })
        let clips = try paths.map({ try NXEClip(source: NXEClipSource(path: $0)) })
        project.visualClips = clips;
        return project
    }
}

class ProjectTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
    }
    
    override func tearDown() {
        self.continueAfterFailure = true
        super.tearDown()
    }
    
    func testHeadlessPlay() {
        self.continueAfterFailure = false
        let engine = NXEEngine.instance()!
        engine.TTPrepare()

        let project = try! typicalProject(clipSubPaths: ["video/canal.mp4", "picture/Desert.jpg"])
        engine.setProject(project)
        
        XCTAssert(engine.play() == Int32(ERRORCODE.ERROR_NONE.rawValue))
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 5.0))
        
        var finished = false
        engine.stopAsync { (errorCode) in
            finished = true
            XCTAssert(errorCode == .ERROR_NONE)
        }
        
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }

    /// Assets successful stopAsync while it's already stopped
    func testStopAsyncWhileStopped() {
        self.continueAfterFailure = false

        let engine = NXEEngine.instance()!
        engine.TTPrepare()

        let project = try! typicalProject(clipSubPaths: ["video/canal.mp4", "picture/Desert.jpg"])
        engine.setProject(project)
        engine.play()
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 5.0))

        // stop while playing
        engine.stop()

        // stop while already stopped
        var finished = false
        engine.stopAsync { (errorCode) in
            finished = true
            XCTAssert(errorCode == .ERROR_NONE)
        }
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
    func testClipFromBundle() {
        // MARK: In order to succeed NXEClip(), NXEEngine instance() should have been invoked to have CNexVideoEditor instance inside can have a chance to create a CNexProjectManager instance
        // Instead, let NXEClip to trigger NXEEngine instance creation.
        _ = NXEEngine.instance()!
        
        let path = clipsourceUrl.appendingPathComponent("video/canal.mp4").path
        do {
            _ = try NXEClip(source: NXEClipSource(path: path))
        } catch let error {
            XCTFail("Failed open clip a path \(path) error:\(error)")
        }
        
    }
    
    func testClipTrimTime() {
        _ = NXEEngine.instance()!
        let path = clipsourceUrl.appendingPathComponent("video/canal.mp4").path
        
        do {
            let clip = try NXEClip(source: NXEClipSource(path: path))
            clip.setTrim(500, endTrimTime: 1500)
            XCTAssert(clip.vinfo.startTrimTime == 500)
            XCTAssert(clip.vinfo.endTrimTime == clip.durationMs - 1500, "endTrimTime \(clip.vinfo.endTrimTime) != \(clip.durationMs - 1500)")
            // MARK: headTrimTime must be renamed to headTrimDuration
            XCTAssert(clip.vinfo.startTrimTime == 500)
            // MARK: tailTrimTime must be renamed to tailTrimDuration
            XCTAssert(clip.vinfo.endTrimTime == clip.durationMs - 1500, "tailTrimTime \(clip.vinfo.endTrimTime) != \(clip.durationMs - 1500)")
            // MARK: Need a new property: trimEndTime (time from 0, beginning of the clip, to end of the trimmed clip)
            //        XCTAssert(clip.trimDuration == 1000)
        } catch let error {
            XCTFail("Failed open clip a path \(path) error:\(error)")
        }
    }
    func testExport() {
        
        let engine = NXEEngine.instance()!
        
        let project = NXEProject()
        
        var paths: [String] = []
        var clips: [NXEClip] = []
        paths.append(clipsourceUrl.appendingPathComponent("video/canal.mp4").path)
        paths.append(clipsourceUrl.appendingPathComponent("picture/Desert.jpg").path)
        for path in paths {
            // MARK: better have a common protocol for NXEVisualClip and NXEAudioClip and remove NXEClip
            do {
                let clip = try NXEClip(source: NXEClipSource(path: path))
                if clip.clipType == NXEClipType.CLIPTYPE_VIDEO {
                    if clip.durationMs > 2000 {
                        clip.setTrim(500, endTrimTime: 2500)
                    }
                } else if clip.clipType == NXEClipType.CLIPTYPE_IMAGE {
                    clip.setImageClipDuration(2000)
                } else if clip.clipType == NXEClipType.CLIPTYPE_AUDIO {
                    if clip.ainfo.totalTime > 2000 {
                        clip.setTrim(500, endTrimTime: clip.durationMs - 1500)
                    }
                }
                clips.append(clip)
            } catch let error {
                XCTFail("Failed open clip a pat \(path) error:\(error)")
            }
        }
        project.visualClips = clips;
        // MARK: This should be avoided just to get total time
        project.update()
        
        XCTAssert(project.getTotalTime() <= 4000, "Project duration \(project.getTotalTime()) > 4000")
        engine.setProject(project)
        
        let width = 1080
        let height = 720
        let bitrate = Int(Double(width * height * 30) * 0.17)
        
        var path = NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true)[0]
        path = (path as NSString).appendingPathComponent("export.mp4")
        try? FileManager.default.removeItem(atPath: path)
        
        var finished = false
        engine.setCallbackWithEncodingProgress({ (percent) in
            print(percent)
        }, encodingEnd:{ (error) in
            XCTAssert(error == nil, "Export finished with error \(error!.localizedDescription)")
            finished = true
        })
        let ret = engine.exportProject(path, width: 1080, height: 720, bitrate: Int32(bitrate), maxFileSize: LONG_MAX, projectDuration: 0, forDurationLimit: false, rotationFlag: ._ROTATION_0)
        XCTAssert(ret == Int32(ERRORCODE.ERROR_NONE.rawValue))
        
        var timeout: TimeInterval = 5.0
        while !finished && timeout > 0 {
            let timeslice: TimeInterval = 0.5
            RunLoop.current.run(until: Date(timeIntervalSinceNow: timeslice))
            timeout -= timeslice
        }
        XCTAssert(finished, "Export not finished within time: \(timeout)")
        
        do {
            let clip = try NXEClip(source: NXEClipSource(path: path))
            let upperBound = 4000 * 1.1
            let lowerBound = 4000 * 0.9
            XCTAssert(Double(clip.durationMs) <= upperBound, "Exported clip duration \(clip.durationMs) > \(upperBound)")
            XCTAssert(Double(clip.durationMs) >= lowerBound, "Exported clip duration \(clip.durationMs) < \(lowerBound)")
        } catch let error {
            XCTFail("Failed open exported video as a clip \(error.localizedDescription)")
        }
    }
    
    func testNullContextAfterExportRequest() {
        let engine = NXEEngine.instance()!
        let project = NXEProject()
        
        let clipPath = clipsourceUrl.appendingPathComponent("picture/Desert.jpg").path
        do {
            let clip = try NXEClip(source: NXEClipSource(path: clipPath))
            project.visualClips = [clip];
            project.update()
            engine.setProject(project)
        } catch let error {
            XCTFail("Failed open clip a pat \(clipPath) error:\(error)")
        }
        
        let width: Int32 = 1080
        let height: Int32 = 720
        let bitrate = Int(Double(width * height * 30) * 0.17)
        
        var path = NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true)[0]
        path = (path as NSString).appendingPathComponent("export.mp4")
        
        var finished = false
        engine.setCallbackWithEncodingProgress({ (percent) in
            print(percent)
        }, encodingEnd:{ (error) in
            XCTAssert(error == nil, "Export finished with error \(error!.localizedDescription)")
            finished = true
        })
        
        var params = NXEExportParamsMake(width, height)
        params.bitPerSec = Int32(bitrate)
        params.maxFileSizeByte = LONG_MAX
        params.durationMs = 0
        params.rotationAngle = ._ROTATION_0
        let ret = engine.export(toPath: path, with: params)
        XCTAssert(ret == Int32(ERRORCODE.ERROR_NONE.rawValue))
        XCTAssert(EAGLContext.current() == nil, "EAGLContext should not be intact after export request")
        
        var timeout: TimeInterval = 5.0
        while !finished && timeout > 0 {
            let timeslice: TimeInterval = 0.5
            RunLoop.current.run(until: Date(timeIntervalSinceNow: timeslice))
            timeout -= timeslice
        }
        XCTAssert(finished, "Export not finished within time: \(timeout)")
    }
    
    func testPreparedEditorWithSolidClip() {
        let engine = NXEEngine.instance()!
        let clips: [NXEClip] = [NXEClip.newSolidClip("red")]
        let project = NXEProject()
        
        project.visualClips = clips;
        engine.setPreviewWith(preview.layer, previewSize:preview.bounds.size)
        engine.setProject(project)
        engine.TTPrepare()
        
        engine.TTPlay(for:5)
        engine.stopSync { errorcode in }
        
    }
    
    func testPreparedEditorNoProject() {
        let engine = NXEEngine.instance()!
        engine.setPreviewWith(self.preview.layer, previewSize:self.preview.bounds.size)

        measure {
            let editor = NXEEngine.instance()!
            
            var prepared = false
            editor.preparedEditor {
                prepared = true
            }
            
            while !prepared {
                RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.1))
            }
        }
    }
    
    func testPreparedEditorEx() {
        for _ in (0...3) {
            autoreleasepool {
                let engine = NXEEngine.instance()!
                engine.preparedEditor {
                }
            }
            autoreleasepool {
                var prepared = false
                let editor = NXEEngine.instance()!
                editor.preparedEditor {
                    prepared = true
                    let clips: [NXEClip] = [NXEClip.newSolidClip("red")]
                    let project = NXEProject()
                    
                    project.visualClips = clips;
                    editor.setPreviewWith(self.preview.layer, previewSize:self.preview.bounds.size)
                    editor.setProject(project)
                    editor.TTPlay(for:3)
                    editor.stopSync { errorcode in }
                }
                while !prepared {
                    RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
                }
            }
        }
    }
    
    func testSingleImageClip() {
        let editor = NXEEngine.instance()!
        editor.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
        
        let name = "Single image clip"
        let sequence = ClipSequence(path: "ClipSources/picture", clipNames: ["Desert.jpg"], name: name)
        editor.TTExport(sequence: sequence, preview: self.preview, label:self.label)
    }
}
