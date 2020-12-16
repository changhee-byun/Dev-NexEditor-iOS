/*
 * File Name   : TemplatePickerPresenter.swift
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
//import NexEditorFramework

class TemplatePickerPresenter: NSObject, ItemPickerDelegate {

    let aspectTypes: [NXEAspectType]
    lazy var dataSource: TemplatePickerDataSource = {
        return TemplatePickerDataSource(aspectTypes: self.aspectTypes)
    }()
    
    @objc public var didEnd: ((_ picker: ItemPickerCollectionViewController, _ assetItem: NXEAssetItem?)->Void)? = nil
    
    // templatePickerViewController with 'templateCell' reuse id in AssetPickers storyboard
    static let source = (storyboardName: "AssetPickers", viewControllerId: "templatePickerViewController", cellReuseId: "templateCell")
    
    @objc override convenience init() {
        self.init(aspectTypes: TemplatePickerDataSource.allAspectTypes)
    }
    
    @objc convenience init(aspectType: NXEAspectType) {
        self.init(aspectTypes: [aspectType])
    }
    
    init(aspectTypes: [NXEAspectType]) {
        self.aspectTypes = aspectTypes;
        super.init()
    }
    
    /// Present picker by pushing to navigationController of viewController. Dismiss by popping.
    @objc func presentFrom(_ viewController: UIViewController, beforePresenting: (_ picker: ItemPickerCollectionViewController)->Void) {
        let source = type(of:self).source
        let storyboard = UIStoryboard(name: source.storyboardName, bundle: nil)
        let destination = storyboard.instantiateViewController(withIdentifier: source.viewControllerId) as! ItemPickerCollectionViewController
        destination.override(reuseId: source.cellReuseId)
        destination.dataSource = self.dataSource
        
        beforePresenting(destination)
        if destination.delegate == nil && didEnd != nil {
            destination.delegate = self
        }
        viewController.navigationController?.pushViewController(destination, animated: true)
    }
    
    func itemAt(indexPath: IndexPath) -> NXEAssetItem? {
        var result: NXEAssetItem? = nil
        if let assetPickerItem = self.dataSource.item(at: indexPath) as? AssetPickerItem {
            result = assetPickerItem.assetItem
        }
        return result
    }
    
    // MARK: - ItemPickerDelegate
    func itemPickerDidCancel(_ picker: ItemPickerController) {
        didEnd?(picker as! ItemPickerCollectionViewController, nil)
        didEnd = nil
    }
    
    func itemPicker(_ picker: ItemPickerController, didSelect indexPath: IndexPath) {
        didEnd?(picker as! ItemPickerCollectionViewController, itemAt(indexPath: indexPath))
        didEnd = nil
    }
}
