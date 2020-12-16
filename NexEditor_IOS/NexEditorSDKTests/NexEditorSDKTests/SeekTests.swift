//
//  SeekTests.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 5/17/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class SeekTests: XCTestCase {
    
    var view: UIView!
    
    override func setUp() {
        super.setUp()
        view = preview
        
        NexLogger.setLogLevel(0, for: .error)
        NexLogger.setLogLevel(0, for: .debug)
    }
    
    override func tearDown() {
        NexLogger.setLogLevel(LogLevelDisabled, for: .error)
        NexLogger.setLogLevel(LogLevelDisabled, for: .debug)

        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testSeekToCurrentShouldCallback() {
        let engine = NXEEngine.instance()!
        engine.setPreviewWith(view.layer, previewSize: view.bounds.size)
        
        let project = NXEProject()
        
        let path = clipsourceUrl.appendingPathComponent("video/canal.mp4").path
        do {
            let clip = try NXEClip(source: NXEClipSource(path: path))
            project.visualClips = [clip];
            engine.setProject(project)
            
            engine.TTPrepare()
            engine.TTPlay(for: 2.0)
            engine.pause()
            
            let position = engine.getCurrentPosition()
            
            // Seek to current position #1
            var finished = false
            engine.setCallbackWithSetTime() { time in
                finished = true
            }
            engine.seek(position)
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 2.0))
            XCTAssert(finished, "Seek to \(position) no callback")
            
            // Seek to current position #2
            finished = false
            engine.setCallbackWithSetTime() { time in
                finished = true
            }
            engine.seek(position)
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 2.0))
            XCTAssert(finished, "Seek to \(position) second time no callback")
            
            engine.stop()
        } catch let error {
            XCTFail("Failed open clip a pat \(path) error:\(error)")
        }
        
    }
    
    func testSeekAndCaptureTwice() {
  
        let engine = NXEEngine.instance()!
        engine.setPreviewWith(view.layer, previewSize: view.bounds.size)
        
        let project = NXEProject()

        let path = clipsourceUrl.appendingPathComponent("video/tvc30sec.mp4").path
        do {
            let clip = try NXEClip(source: NXEClipSource(path: path))
            project.visualClips = [clip];
            engine.setProject(project)
            
            engine.TTPrepare()
            engine.TTPlay(for: 2.0)
            engine.pause()
            
            let captureSpot = engine.getCurrentPosition() + 500

            // FIXME: First capture is not from the current preview
            _ = engine.TTCaptureFrame(at: captureSpot)

            let image1 = engine.TTCaptureFrame(at: captureSpot)
            XCTAssert(image1 != nil, "Image capture 1 failed")
            
            // Seek and Capture again
            let image2 = engine.TTCaptureFrame(at: captureSpot)
            XCTAssert(image2 != nil, "Image capture 2 failed")
            
            // Compare captured images
            let data1 = UIImagePNGRepresentation(image1!)
            let data2 = UIImagePNGRepresentation(image2!)
            XCTAssert(data1 == data2, "Image captures are differernt")
            
            engine.stop()
        } catch let error {
            XCTFail("Failed open clip a pat \(path) error:\(error)")
        }
    }
    
    func testSetLutAndCompareCapture() {
        // NESI-372
        let engine = NXEEngine.instance()!
        engine.setPreviewWith(view.layer, previewSize: view.bounds.size)
        
        let project = NXEProject()
        
        let path = clipsourceUrl.appendingPathComponent("video/tvc30sec.mp4").path
        do {
            let clip = try NXEClip(source: NXEClipSource(path: path))
            project.visualClips = [clip];
            engine.setProject(project)
            
            engine.TTPrepare()
            engine.TTPlay(for: 5.0)
            engine.pause()
            
            let captureSpot = engine.getCurrentPosition() + 500
            
            _ = engine.TTCaptureFrame(at: captureSpot)
            
            let image1 = engine.TTCaptureFrame(at: captureSpot)
            XCTAssert(image1 != nil, "Image capture 1 failed")
            
            clip.setLut(NXELutTypes.LUT_DISNEY)
            engine.updatePreview()
            
            // Seek and Capture again
            let image2 = engine.TTCaptureFrame(at: captureSpot)
            XCTAssert(image2 != nil, "Image capture 2 failed")
            
            // Compare captured images
            let data1 = UIImagePNGRepresentation(image1!)
            let data2 = UIImagePNGRepresentation(image2!)
            XCTAssert(data1 != data2, "Image captures should be differernt")
            
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 1.0))
            
            engine.stop()
        } catch let error {
            XCTFail("Failed open clip a pat \(path) error:\(error)")
        }
    }
    
    
    /// Create two engines, deallocate first engine and returns the secondly created engine
    func engineWeirdWay() -> NXEEngine {
        var holder: [NXEEngine] = []
        autoreleasepool {
            let firstEngine = NXEEngine.instance()
            holder.append(NXEEngine.instance())
            print("Good bye first engine \(firstEngine!)")
        }
        return holder[0]
    }
    
    /// NESI-375 crash if first engine instace deallocated
    func testFirstEngineDeallocated() {
        let engine = engineWeirdWay()
        engine.setPreviewWith(view.layer, previewSize: view.bounds.size)
        
        let project = NXEProject()
        
        let path = clipsourceUrl.appendingPathComponent("video/canal.mp4").path
        do {
            let clip = try NXEClip(source: NXEClipSource(path: path))
            project.visualClips = [clip];
            engine.setProject(project)
            
            /// Crash before NESI-375 fix due to KMLayer delegate is dangling
            engine.TTPrepare()
            engine.TTPlay(for: 2.0)
            engine.pause()
            
            let seekSpot = engine.getCurrentPosition() + 500
            engine.seek( seekSpot )
            /// Crash before NESI-375 fix: Successive seek attemps dangling reference to seekDelegate
            engine.TTSeek(seekSpot + 100)

            RunLoop.current.run(until: Date(timeIntervalSinceNow: 1.0))
            
            engine.stop()
        } catch let error {
            XCTFail("Failed open clip a pat \(path) error:\(error)")
        }
    }
    
    func testSeek2TimesBeforePlayingShortClipWithTransition() {
        InternalAssetLibrary().removeAllAssetPackageURLs()
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("SampleAssets.bundle/packages"))
        
        let engine = NXEEngine.instance()!
        engine.TTPrepare()
        engine.setPreviewWith(view.layer, previewSize: view.bounds.size)
        
        let project = NXEProject()
        
        let names = ["tvc3sec.mp4", "canal.mp4"]
        let paths = names.map({ clipsourceUrl.appendingPathComponent("video/\($0)").path})
        
        do {
            let clips = try paths.map( { try NXEClip(source: NXEClipSource(path: $0))});
            clips[0].transitionEffectID = "com.nexstreaming.kinemaster.builtin4.starwipe"
            clips[0].setTransitionEffectDuration(2000)
            project.visualClips = clips
            engine.setProject(project)
            
            engine.resolveProject(true)
            
            // Confuse the engine by seeking a few just before starting playback
            engine.videoEditor.setTime(3000, display: 1, idrFrame: 0)
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.3))
            engine.videoEditor.setTime(0, display: 1, idrFrame: 0)
            engine.videoEditor.startPlay(0)
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 5.0))
            engine.stop()
        } catch let error {
            XCTFail("Failed open clip paths \(paths) error:\(error)")
        }
    }

}
