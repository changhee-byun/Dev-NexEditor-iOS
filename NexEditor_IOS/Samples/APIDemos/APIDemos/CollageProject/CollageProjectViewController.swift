/*
 * File Name   : CollageProjectViewController.swift
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
import NexEditorFramework

/*
 Collage API Improvements
 - NXEEngine.setProject to regard Collect Project's Aspect Ratio
 - Aspect Ratio Change to reconfigure preview layer, without having to call setPreviewWith(_, previewSize:) again
 - Slot Configuration Copy? when swiching collage item with different number of slots
 */

private let cellReuseId = "cell"

extension Playable {
    var isStoppedAtAlmostEnd: Bool {
        return (!status.playing) && (duration > CMTime.zero) && ( abs(currentTime.seconds - duration.seconds) < 0.5)
    }
}

extension NXEAssetItemGroup {
    func itemWith(slotCount count: Int, nearest: Bool) -> NXECollageAssetItem? {
        if items.count == 0 { return nil }
        
        var result: NXECollageAssetItem? = nil
        
        let filtered = items.filter({ (($0 as? NXECollageAssetItem)?.info.slots.count ?? 0) == count})
        result = filtered.first as? NXECollageAssetItem
        
        if result == nil && nearest {
            let sorted = (items as! [NXECollageAssetItem]).sorted(by: { $0.info.slots.count < $1.info.slots.count})
            result = (count == 0) ? sorted.first : sorted.last
        }
        return result
    }
}

class CollageProjectViewController: UIViewController {

    @IBOutlet var preview: UIView!
    @IBOutlet var collageEditView: CollageEditView!
    @IBOutlet var playPauseButton: UIButton!
    @IBOutlet var labelProgress: UILabel!
    @IBOutlet var labelTotal: UILabel!
    @IBOutlet var slider: UISeekSlider!
    @IBOutlet var collectionView: UICollectionView!
    @IBOutlet var tabBar: UITabBar!
    @IBOutlet var tabClips: UITabBarItem!
    @IBOutlet var tabCollages: UITabBarItem!
    @IBOutlet var tabItems: UITabBarItem!

    fileprivate var clipsSelection: MediaPickerSelection?
    fileprivate var selectedCollage: NXEAssetItemGroup? {
        willSet(new) {
            self.title = new?.localizedName ?? "Select Collage"
        }
    }
    fileprivate var selectedItem: NXECollageAssetItem?
    
    fileprivate var playable: Playable!
    fileprivate var editor: NXEEngine!
    
    private var projectTabBarController: CollageProjectTabBarController!
    
    private lazy var imageAdd: UIImage = { return UIImage(named:"icon-media")! }()
    
    private lazy var mediaPickerPresenter: MediaPickerPresenter = {
        return MediaPickerPresenter() { [weak self] in
            self?.dismiss(animated: true, completion: nil)
        }
    }()
    
    // MARK: -
    override func viewDidLoad() {
        super.viewDidLoad()

        self.playPauseButton.isEnabled = false
        self.tabItems.isEnabled = false
        self.title = "Select Collage"
        
        editor = NXEEngine.instance()
        editor.setPreviewWith(preview.layer, previewSize: preview.bounds.size)
        
        // Setup monitoring playback status for UI updates
        playable = NexEditorPlayable(editor: editor)
        playable.addObserver(slider)
        playable.addObserver(AdhocPlayableStateObserver({
            [weak self] (playable, state, status) in
            if state == .playing {
                let imageNameDefault = status.playing ? "pause_default" : "play_default"
                self?.playPauseButton.setImage(UIImage(named:imageNameDefault), for: .normal)
            }
            if state == .loaded {
                self?.playPauseButton.isEnabled = status.loaded
                self?.labelTotal.text = StringTools.formatTime(interval: TimeInterval(playable.duration.seconds))
                let time = status.loaded ? 0 : playable.currentTime.seconds
                self?.labelProgress.text = StringTools.formatTime(interval: time)
            }
            
            if state == .currentTime {
                self?.labelProgress.text = StringTools.formatTime(interval: TimeInterval(playable.currentTime.seconds))
            }
        }))
        slider.onSeek = { [weak self] (time) in
            self?.playable.seek(to: time)
        }
        
        // Tab Bar Controller Setup
        let controller = CollageProjectTabBarController(self.tabBar,
                                                        self.collectionView,
                                                        cellReuseId: cellReuseId,
                                                        tabBarItems: [.clips: tabClips,
                                                                      .collages: tabCollages,
                                                                      .items: tabItems])
        
        controller.onItemSelected = { [weak self ] (controller, tab, item) in
            guard let me = self else { return }
            if tab == .clips {
                if item is UIImage && ((item as! UIImage) == me.imageAdd) {
                    me.presentMediaPickerForClipSelection()
                }
            } else if tab == .collages {
                guard let collage = item as? NXEAssetItemGroup else { return }
                me.changeSelectedCollage(collage)
            } else if tab == .items {
                guard let item = item as? NXECollageAssetItem else { return }
                me.changeSelectedItem(item)
            }
        }

        controller.onTabDataSourceLoaderRequired = { [weak self] (controller, tab) in
            return self?.itemLoaderFor(tab: tab) ?? { (i) in return [] }
        }
        controller.onActivityIndicatorRun = { [weak self] (run) in
            guard let me = self else { return }
            me.runWithActivityIndicator(run)
        }
        
        projectTabBarController = controller
        
        editor.preparedEditor {
            controller.selectTab(.clips)
        }
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    override func willMove(toParent parent: UIViewController?) {
        if parent == nil {
            self.navigationController?.setNavigationBarHidden(false, animated: false)
        }
    }
    
    override func viewWillTransition(to size: CGSize, with coordinator: UIViewControllerTransitionCoordinator) {
        super.viewWillTransition(to: size, with: coordinator)

        let selectedRegionIndex = self.collageEditView.selectedRegionIndex
        let currentProjectValid = (self.editor.project as? NXECollageProject != nil)
        if currentProjectValid {
            self.collageEditView.dismissSlotEditView()
        }

        coordinator.animate(alongsideTransition: { (context) in
            let wide = size.width > size.height
            self.switchFullscreen(wide)
        }) { (context) in
            self.editor.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
            if currentProjectValid {
                self.collageEditView.updateRegionPositions()
                if selectedRegionIndex != NSNotFound {
                    self.collageEditView.selectedRegionIndex = selectedRegionIndex
                }
            }
        }
    }

    func switchFullscreen(_ on: Bool) {
        let collectionViewHeight = collectionView.constraints.filter( {$0.identifier == "height"})[0]
        let tabBarHeight = tabBar.constraints.filter( {$0.identifier == "height"})[0]
        if on {
            collectionViewHeight.constant = 0
            tabBarHeight.constant = 0
        } else {
            collectionViewHeight.constant = 100
            tabBarHeight.constant = 49
        }
        self.navigationController?.setNavigationBarHidden(on, animated: true)
    }
    
    func presentMediaPickerForClipSelection() {
        mediaPickerPresenter.present(from: self) { [weak self] (selection) in
            guard let me = self else { return }
            me.clipsSelection = selection
            
            me.projectTabBarController.reloadItems()
            me.runWithActivityIndicator {
                me.setProjectWith(clipSources: selection.clipSources)
            }
        }
    }
    
    // MARK: - Loading Collage Assets and Asset Items
    
    func anyCollage() -> NXEAssetItemGroup? {
        var result: NXEAssetItemGroup? = nil
        let lib = NXEAssetLibrary.instance()
        let groups = lib.groups(inCategory: NXEAssetItemCategory.collage)
        if groups.count > 0 {
            result = groups.first
        }
        return result
    }
    
    // Returns collage loader function for Collages Tab
    func loaderCollages() -> CollageProjectTabBarController.DataSourceLoader {
        let collage = selectedCollage
        return { (selectedIndex) in
            let lib = NXEAssetLibrary.instance()
            let groups = lib.groups(inCategory: NXEAssetItemCategory.collage)
            if let collage = collage {
                selectedIndex = groups.index(where: {$0.localizedName == collage.localizedName })
            }
            return groups
        }
    }
    
    // Returns collage item loader function for Items Tab
    func loaderCollageItems() -> CollageProjectTabBarController.DataSourceLoader {
        let loader: CollageProjectTabBarController.DataSourceLoader
        if let collage = selectedCollage {
            let selectedItem = self.selectedItem
            loader = { (selectedIndex) in
                let items = (collage.items as! [NXECollageAssetItem]).sorted(by: { $0.info.slots.count < $1.info.slots.count})
                if let item = selectedItem {
                    selectedIndex = items.index(where: { $0.itemId == item.itemId })
                }
                return items
            }
        } else {
            loader = { (i) in return [] }
        }
        
        return loader
    }
    
    // Returns clip loader function for Clips Tab
    func loaderClips() -> CollageProjectTabBarController.DataSourceLoader {
        let imageAdd = self.imageAdd
        let loadables = clipsSelection?.mediaItems ?? []
        return { (selectedIndex) in
            var clips: [ThumbnailLoadable] = loadables
            clips.append(imageAdd)
            return clips
        }
    }
    
    func itemLoaderFor(tab: CollageProjectTabBarController.Tab) -> CollageProjectTabBarController.DataSourceLoader {
        let loadItems: CollageProjectTabBarController.DataSourceLoader
        
        if tab == .clips {
            loadItems = loaderClips()
        } else if tab == .collages {
            loadItems = loaderCollages()
        } else if tab == .items {
            loadItems = loaderCollageItems()
        } else {
            loadItems = { (i) in return [] }
        }
        
        return loadItems
    }

    // MARK: - Building project with clip sources and a collage asset item
    
    /// Change selected item and build a new project
    func changeSelectedItem(_ item: NXECollageAssetItem) {
        self.selectedItem = item
        
        let clipSources = self.clipsSelection?.clipSources ?? []
        self.runWithActivityIndicator {
            self.setProjectWith(clipSources: clipSources,
                                collageItem: self.selectedItem!)
        }
    }
    
    /// Change selected collage, item, and build a new project accordingly.
    func changeSelectedCollage(_ collage: NXEAssetItemGroup) {
        self.selectedCollage = collage
        self.tabItems.isEnabled = true
        
        let clipSources = self.clipsSelection?.clipSources ?? []
        
        // Choose collage item based on the slot count
        let slotCount = self.selectedItem?.info.slots.count ?? clipSources.count
        if let item = collage.itemWith(slotCount: slotCount, nearest: true)  {
            changeSelectedItem(item)
        } else {
            self.alert(.error, "Invalid Collage Asset without items")
        }
    }

    func setProjectWith(clipSources: [NXEClipSource], collageItem: NXECollageAssetItem) {
        
        playable.pause()
        playable.changeState(.loaded, to: false)

        do {
            let project = try NXECollageProject(collageAssetItem: collageItem)
            
            let clips = try clipSources.map({ try NXEClip(source: $0 )})
            
            for (index, slot) in project.slots.enumerated() {
                let clip: NXEClip
                if index >= clips.count {
                    clip = NXEClip.newSolidClip("#00000000")
                } else {
                    clip = clips[index]
                }
                slot.clip = clip
            }
            
            editor.setProject(project)
            // each collage item has it's own aspect ratio
            let ratio = NXESizeInt(width: project.info.aspectRatio.width, height: project.info.aspectRatio.height)
            editor.setAspectType(.custom, withRatio: ratio)
            // aspect ratio change requires setting preview layer again
            editor.setPreviewWith(preview.layer, previewSize: preview.bounds.size)
            let editTime = CMTime(seconds: Double(project.info.editTimeMs) / 1000.0, preferredTimescale: 1000)
            playable.seek(to: editTime)

            playable.changeState(.loaded, to: true)
            
            collageEditView.setProject(project, preview: self.preview)
            collageEditView.onSlotEdit = { [weak self] (slot, index) in
                let editing = slot != nil
                let tintColor = editing ? UIColor.gray : nil
                let tintAdjustmentMode: UIView.TintAdjustmentMode = editing ? .dimmed : .normal
                self?.tabBar.isUserInteractionEnabled = !editing
                self?.tabBar.tintAdjustmentMode = tintAdjustmentMode
                self?.tabBar.tintColor = tintColor
                self?.collectionView.isUserInteractionEnabled = !editing
                self?.collectionView.tintAdjustmentMode = tintAdjustmentMode
                self?.collectionView.tintColor = tintColor
            }
        } catch let e {
            self.alert(.error, e.localizedDescription)
        }
    }
    
    func setProjectWith(clipSources: [NXEClipSource]) {
        if let item = selectedItem {
            setProjectWith(clipSources: clipSources, collageItem: item)
        } else {
            // no item, no collage. pick a random collage if not yet selected one.
            if let collage = anyCollage() {
                changeSelectedCollage(collage)
            } else {
                alert(.error, "Not Collage Assets available")
                return
            }
        }
    }

    // MARK: - Storyboard Actions
    @IBAction func actionPlayPause(_ sender: Any) {
        let playing = playable.status.playing
        if playing {
            playable.pause()
        } else {
            if playable.isStoppedAtAlmostEnd {
                playable.seek(to: CMTime.zero)
            }
            playable.play()
        }
    }
}

// MARK: - Convenience Extensions

extension ClipMediaItem: ThumbnailLoadable { }

extension UIImage: ThumbnailLoadable {
    func loadThumbnail(size: CGSize, _ loaded: @escaping (UIImage?) -> Void) {
        loaded(self.resized(size))
    }
}

extension NXEAssetItemGroup: ThumbnailLoadable {
    func loadThumbnail(size: CGSize,_ loaded: @escaping(_ image: UIImage?)->Void ) {
        loadIconImageData { (data, ext) in
            var image: UIImage? = nil
            if data != nil && (ext == "png" || ext == "jpg") {
                image = UIImage(data: data!)?.resized(size)
            }
            loaded(image)
        }
    }
}

extension NXEAssetItem: ThumbnailLoadable {
    func loadThumbnail(size: CGSize,_ loaded: @escaping(_ image: UIImage?)->Void ) {
        loadIconImageData { (data, ext) in
            var image: UIImage? = nil
            if data != nil && (ext == "png" || ext == "jpg") {
                image = UIImage(data: data!)?.resized(size)
            }
            loaded(image)
        }
    }
}
