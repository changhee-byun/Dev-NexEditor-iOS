/**
 * File Name   : ViewController.swift
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

import UIKit
import NexEditorFramework
import DKImagePickerController
import Photos


enum PlaybackButtonType {
    case play
    case pause
}

extension Playable {
    var isStoppedAtAlmostEnd: Bool {
        return (!status.playing) && (duration > kCMTimeZero) && ( abs(currentTime.seconds - duration.seconds) < 0.5)
    }
}
class BasicEditorViewController: UIViewController {

    @IBOutlet var preview: NXELayerEditorView!
    @IBOutlet var collectionViewClips: UICollectionView!
    
    @IBOutlet var toolbar: UIToolbar!
    @IBOutlet var buttonPlayback: UIButton!
    @IBOutlet var buttonText: UIButton!
    @IBOutlet var buttonItemExport: UIBarButtonItem!
    @IBOutlet var slider: UISeekSlider!

    // MARK: -
    
    var projectHolder: EditorProjectHolder!
    
    var editor: NXEEngine {
        return projectHolder.editor
    }
    var playable: Playable!

    var clipsDataSource: ClipsDataSource!
    
    // MARK: -
    
    var settings = SettingsViewController.Settings() {
        willSet(newSettings) {
            if settings.aspectRatio != newSettings.aspectRatio {
                let type = newSettings.aspectRatio.type
                let editor = projectHolder.editor
                if type == .custom {
                    editor.setAspectType(type, withRatio: newSettings.aspectRatio)
                } else {
                    editor.aspectType = type
                }
                editor.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
                
                // refresh layers
                let layers = self.projectHolder.layers
                self.projectHolder.layers = layers
            }
        }
    }
    
    // MARK: -
    var playbackButtonType: PlaybackButtonType = .play {
        willSet(newType) {
            switch newType {
            case .play:
                buttonPlayback.setImage(#imageLiteral(resourceName: "icon-play"), for: .normal)
                break
                
            case .pause:
                buttonPlayback.setImage(#imageLiteral(resourceName: "icon-pause"), for: .normal)
                break
            }
        }
    }

    let clipCellReuseId = "thumbnail"
    // MARK: -
    override func viewDidLoad() {
        super.viewDidLoad()
        
        projectHolder = EditorProjectHolder(stateChanged: { [weak self] clipsReady in
            guard let me = self else { return }

            me.playable.changeState(.loaded, to: clipsReady)

            me.clipsDataSource = ClipsDataSource(items: me.projectHolder.clipSources, cellReuseId: me.clipCellReuseId)
            me.collectionViewClips.dataSource = me.clipsDataSource
            me.collectionViewClips.delegate = me.clipsDataSource.collectionViewDelegate
            me.collectionViewClips.reloadData()
        })
        editor.setPreviewWith(preview.layer, previewSize: preview.bounds.size)
        
        playable = NexEditorPlayable(editor: editor)
        playable.addObserver(slider)
        playable.addObserver(AdhocPlayableStateObserver({
            [weak self] (playable, state, status) in
            if state == .playing {
                self?.playbackButtonType = status.playing ? .pause : .play
            }
            if state == .loaded {
                self?.buttonPlayback.isEnabled = status.loaded
                self?.buttonItemExport.isEnabled = status.loaded
            }
        }))
        
        slider.onSeek = { [weak self] (time) in
            self?.playable.seek(to: time)
        }
        
        buttonPlayback.isEnabled = false
        buttonItemExport.isEnabled = false
        
        let textLayer = NXETextLayer(text: "Hello", font: nil, point: CGPoint(x:-1, y:-1))!
        self.projectHolder.layers = [textLayer]
        
        clipsDataSource = ClipsDataSource(items: projectHolder.clipSources, cellReuseId: clipCellReuseId)
        collectionViewClips.dataSource = clipsDataSource
        collectionViewClips.delegate = clipsDataSource.collectionViewDelegate
        
        preview.layerEditor = CustomLayerEditor()
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    override func viewDidLayoutSubviews() {
        editor.setPreviewWith(preview.layer, previewSize: preview.bounds.size)
        
        let height = collectionViewClips.constraints.filter( {$0.identifier == "height"})[0]
        if view.bounds.size.width > view.bounds.size.height {
            height.constant = 0
        } else {
            height.constant = 74
        }
    }

    // MARK: -

    /// Present image/video picker to create a new project with selected clips
    @IBAction func add(_ sender: Any) {
        let pickerController = DKImagePickerController()
        
        pickerController.didSelectAssets = { (assets: [DKAsset]) in

            var phassets : [PHAsset] = []
            for asset in assets {
                if let phasset = asset.originalAsset {
                    phassets.append(phasset)
                }
            }
            self.projectHolder.clipSources = phassets
        }
        
        // pause if playing
        playable.pause()
        
        present(pickerController, animated: true)
    }
    
    @IBAction func addText(_ sender: UIButton) {
        self.alertTextInput("Enter text for new layer") { (text) in
            if let text = text {
                let layer = NXETextLayer(text: text, font: nil, point: CGPointNXELayerCenter)!
                var layers = self.projectHolder.layers
                layers.append(layer)
                self.projectHolder.layers = layers
                
                layer.startTime = self.editor.getCurrentPosition()
                layer.endTime = layer.startTime + layer.endTime
                layer.scaledFrame = CGRect(x: 0.1, y: 0.7, width: 0.3, height: 0.2)
            }
        }
    }
    
    @IBAction func playPause(_ sender: Any) {
        let playing = playable.status.playing
        if playing {
            playable.pause()
        } else {
            if playable.isStoppedAtAlmostEnd {
                playable.seek(to: kCMTimeZero)
            }
            playable.play()
        }
    }
    
    /// Prompts export options and begin exporting
    @IBAction func export(_ sender: Any) {
        
        let options = projectHolder.exportOptions()
        
        self.alertActions("Export",
                          message: "Select format",
                          actionTitles:options.titles,
                          dismissTitle:"Cancel")
        { actionIndex in
            if let actionIndex = actionIndex {
                let width = options.widths[actionIndex]
                let height = options.heights[actionIndex]
                
                self.export("export.mp4", width: Int32(width), height: Int32(height)) { complete, path in
                    if !complete {
                        self.alert("Information", message: "Export cancelled")
                    }
                }
            }
        }
    }
    
    // MARK: -
    
    /// Export the current project to the documents directory with the specified name. Displays progress alert with a Cancel button.
    func export(_ filename: String, width: Int32, height: Int32, finished:@escaping (_ complete: Bool, _ path: String?)-> Void) {
        let alert = self.alertSimple("Exporting", message:"...", dismissTitle: "Cancel") {
            // dismissed
            self.editor.stopSync({ (errorcode) in
            })
            finished(false, nil)
        }
        let path = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0].appendingPathComponent(filename).path
        
        self.editor.setCallbackWithEncodingProgress(
            { [weak alert](progress) in
                alert?.message = "In progress: \(progress)%"
            },
            encodingEnd: { [weak self] (error) in
                self?.dismiss(animated: true, completion: nil)
                finished(true, path)
        })
        self.editor.exportProject(path, width: Int32(width), height: Int32(height),
                                  bitrate: 1000000,
                                  maxFileSize: LONG_MAX,
                                  projectDuration: 0,
                                  forDurationLimit: false,
                                  rotationFlag: NXERotationAngle._ROTATION_0)
        self.present(alert, animated:true)
    }
    
  
    // MARK: -
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "settings" {
            self.editor.pause()
            let destination = segue.destination as! SettingsViewController
            destination.settings = self.settings
            destination.done = { settings in
                self.settings = settings
                if self.projectHolder.clipSources.count > 0 {
                    let sources = self.projectHolder.clipSources
                    self.projectHolder.clipSources = sources
                }
            }
        }
    }
}

class ClipsDataSource: ThumbnailSequenceDataSource {
    lazy var collectionViewDelegate: UICollectionViewDelegate = {
        return ThumbnailSequenceCollectionViewDelegate(dataSource: self)
    }()
}
