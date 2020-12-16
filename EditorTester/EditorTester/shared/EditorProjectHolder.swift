/**
 * File Name   : EditorProjectHolder.swift
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
//import NexEditorFramework
import Photos

class EditorProjectHolder {
    
    var stateChanged: (_ clipsRady: Bool) -> Void
    
    let editor: NXEEngine
    var project: NXEProject? {
        willSet(newProject) {
            self.editor.setProject(newProject)
        }
        didSet(oldProject) {
            self.editor.preparedEditor({
                self.stateChanged(true)
            })
        }
    }
    
    var layers: [NXELayer] = [] {
        willSet(newLayers) {
            if self.project != nil {
                self.project!.layers = newLayers
            }
        }
    }
    
    var clips: [NXEClip] = [] {
        didSet(oldClips) {
            if clips.count > 0 {
                let project = NXEProject()
                project.visualClips = clips
                project.layers = layers
                project.update()
                self.project = project
            }
        }
    }
    
    var clipSources: [ClipSource] = [] {
        willSet(newSources) {
            stateChanged(false)
            
        }
        
        didSet(oldSources) {
            var clips: [NXEClip] = []
            
            for source in clipSources {
                do {
                    let clip = try NXEClip(source: source.nxeClipSource)
                    clips.append(clip)
                } catch let error {
                    print(error.localizedDescription)
                }
            }
            self.clips = clips
        }
    }
    
    init(stateChanged: @escaping (_ clipsRady: Bool) -> Void) {
        self.stateChanged = stateChanged
        self.editor = NXEEngine.instance()
    }
    
    func exportOptions() -> (titles: [String], widths: [Int], heights: [Int]) {
        var landscape = true
        
        var titles: [String] = ["Low", "Medium", "High"]
        var heights: [Int] = [ 360, 720, 1080 ]
        var widths: [Int] = []
        
        let ratio = editor.aspectRatio
        let scale: Float
        if ratio.width < ratio.height {
            scale = Float(ratio.height) / Float(ratio.width)
            landscape = false
        } else {
            scale = Float(ratio.width) / Float(ratio.height)
        }
        
        for height in heights {
            widths.append(Int(Float(height) * scale))
        }
        
        if !landscape {
            let hold = widths;
            widths = heights;
            heights = hold
        }
        
        for index in 0...titles.count-1 {
            titles[index] = "\(titles[index]) - (\(widths[index]) x \(heights[index]))"
        }
        
        return (titles: titles, widths: widths, heights: heights)
    }
    
}
