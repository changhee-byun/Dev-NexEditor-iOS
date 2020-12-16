/**
 * File Name   : IntroViewController.swift
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
import UIKit
import NexEditorFramework

enum MenuItem: String {
    case preview = "Create Movie"
    case collage = "Collage Project"
    case beattemplate = "Beat Template"
    case exportSetting = "Export Setting"
    case assets = "Asset Item Catalog"
    case packages = "Asset Packages"
    case sdkVersion = "SDK Version"
}

typealias MenuItemAction = (_ item: MenuItem, _ viewController: UIViewController) -> Void

protocol MenuItemSegueHandler {
    var segueId: String { get }
    func prepareSegue(_ segue: UIStoryboardSegue, sender: Any?)
}

struct MenuItemOptions {
    var action: MenuItemAction
    var segueHandler: MenuItemSegueHandler?
    
    init(action: @escaping MenuItemAction,
         segueHandler: MenuItemSegueHandler? = nil)
    {
        self.action = action
        self.segueHandler = segueHandler
    }
}

class CollageMenuItemSegueHandler: MenuItemSegueHandler {
    
    let segueId: String
    
    init(segueId: String)
    {
        self.segueId = segueId
    }
    
    func prepareSegue(_ segue: UIStoryboardSegue, sender: Any?)
    {
        //let destination = segue.destination as! CollageProjectViewController
    }
}

extension MenuItem {
    static let performSegueAction: MenuItemAction = { item, viewController in
        if let segueId = item.option.segueHandler?.segueId {
            viewController.performSegue(withIdentifier: segueId, sender: viewController)
        }
    }
    
    static let options: [MenuItem: MenuItemOptions] = [
        /// Segue to segueIdMediaPicker with media picker options
        
    
        .preview: MenuItemOptions(action: performSegueAction,
                                  segueHandler: PreviewMenuItemSegueHandler(.preview,
                                                                            options: MediaPickerOptions(video: true, image: true, audio: false, singleSelect: false, showInfoButton: true), segueId: "mediaPicker", followingSegueId: "seguePreview"
)),
        .collage: MenuItemOptions(action: performSegueAction,
                                  segueHandler: CollageMenuItemSegueHandler(segueId: "collageProject")),
        
        .beattemplate: MenuItemOptions(action: performSegueAction,
                                       segueHandler: PreviewMenuItemSegueHandler(.preview,
                                                                                 options: MediaPickerOptions(video: false, image: true, audio: false, singleSelect: false, showInfoButton: true), segueId: "mediaPicker", followingSegueId: "beattemplate")),
        
        .exportSetting: MenuItemOptions(action: { item, viewController in
            let storyboard = UIStoryboard(name: "AssetPickers", bundle: nil)
            let destination = storyboard.instantiateViewController(withIdentifier: "exportSettingViewController") as! ExportSettingViewController
            destination.dataSource = ExportSettingDataSourceImpl()
            viewController.navigationController?.pushViewController(destination, animated:true)
        }),
        
        .assets: MenuItemOptions(action: { item, viewController in
            let storyboard = UIStoryboard(name: "AssetPickers", bundle: nil)
            let destination = storyboard.instantiateViewController(withIdentifier: "assetCategoryPicker") as! AssetCategoriesViewController
            viewController.navigationController?.pushViewController(destination, animated:true)
        }),
        
        .packages: MenuItemOptions(action: { item, viewController in
            viewController.performSegue(withIdentifier: "assetPackages", sender: viewController)
        }),
        
        /// No media picker option, Show SDK version using a simple alert
        .sdkVersion: MenuItemOptions(action: { item, viewController in }),
        
    ]

    var option: MenuItemOptions {
        return type(of:self).options[self]!
    }
    
    var title: String {
        var result: String
        if self == .sdkVersion {
            result = "SDK Version: \(NXEEngine.sdkVersionString()!)"
        } else {
            result = self.rawValue
        }
        return result
    }
}

class IntroViewController: UITableViewController
{
    static let menuItems: [MenuItem] = [
                                   .preview,
                                   .collage,
                                   .beattemplate,
                                   .exportSetting,
                                   .assets,
                                   .packages,
                                   .sdkVersion]
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.view.isUserInteractionEnabled = false
        runWithBarButtonItemActivityIndicator {
            AssetPackageManager.manager.installAllPackages()
            self.view.isUserInteractionEnabled = true
        }
    }
    
    // MARK: - UITableViewDataSource
    override func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return type(of:self).menuItems.count
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "IntroCell", for: indexPath) as! CommonTypeATableViewCell
        
        cell.titleLabel.text = type(of:self).menuItems[indexPath.row].title
        return cell
    }
    
    // MARK: - UITableViewDelegate
    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        
        let menuItem = type(of:self).menuItems[indexPath.row]
        menuItem.option.action(menuItem, self)
        tableView.deselectRow(at: indexPath, animated: true)
    }
    
    // MARK: -
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        guard let index = self.tableView.indexPathForSelectedRow?.row else {
            return
        }
        
        type(of:self).menuItems[index].option.segueHandler?.prepareSegue(segue, sender: sender)
    }
}
