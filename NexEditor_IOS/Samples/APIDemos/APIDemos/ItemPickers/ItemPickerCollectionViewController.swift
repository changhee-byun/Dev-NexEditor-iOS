/**
 * File Name   : ItemPickerCollectionViewController.swift
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

extension UIView {
    
    /// Find the first subview of 'type'
    /// ```
    /// if let label = view.firstSubview(ofClass: UILabel.self) { ... } 
    /// ```
    func firstSubview<T: UIView>(ofClass type: T.Type) -> T? {
        var result: T! = nil
        
        if let index = subviews.index( where: { $0 is T } ) {
            result = subviews[index] as! T
        }
        return result
        
    }
}

extension UICollectionViewCell: ItemPickerViewCell {
    var textLabel: UILabel? {
        return contentView.firstSubview(ofClass: UILabel.self)
    }
    
    // var imageView: UIImageView? declared in UICollectinoViewImageView.swift
}


struct DefaultItemViewCollectionView: ItemPickerItemView {
    var imageSize: CGSize = CGSize(width: 120, height: 120)
    var reusableCellClassInfo: ReusableCellInfo? = ReusableCellInfo(reuseId: "itemCell", cls: nil)
    
    func render(_ item: ItemPickerItem, with cell: ItemPickerViewCell) {
        if let cell = cell as? UICollectionViewCell {
            cell.textLabel?.text = item.title
            if let imageView = cell.imageView {
                imageView.image = nil
                item.loadImage(size: imageSize, loaded: { (image) in
                    cell.imageView?.image = image
                })
            }
            
            cell.backgroundColor = cell.isSelected ? UIColor.blue : UIColor.clear
        }
    }
    
    /// create a copy with the specified reuseId
    func changing(reuseId: String) -> DefaultItemViewCollectionView {
        let cellInfo = ReusableCellInfo(reuseId: reuseId, cls: self.reusableCellClassInfo!.cls)
        return DefaultItemViewCollectionView(imageSize: self.imageSize, reusableCellClassInfo: cellInfo)
    }
}

@objc protocol ItemPickerObjcDelegate: NSObjectProtocol {
    func itemPicker(_ picker: ItemPickerCollectionViewController,
                    didSelect indexPath: IndexPath)
    func itemPickerDidCancel(_ picker: ItemPickerCollectionViewController)
}

/// Convert ItemPickerObjcDelegate to ItemPickerDelegate
class ItemPickerDelegateConverter: NSObject, ItemPickerDelegate {
    let objcDelegate: ItemPickerObjcDelegate
    
    init(_ objcDelegate: ItemPickerObjcDelegate)
    {
        self.objcDelegate = objcDelegate
        super.init()
    }
    
    func itemPicker(_ picker: ItemPickerController, didSelect indexPath: IndexPath)
    {
        self.objcDelegate.itemPicker(picker as! ItemPickerCollectionViewController, didSelect: indexPath)
    }
    
    func itemPickerDidCancel(_ picker: ItemPickerController)
    {
        self.objcDelegate.itemPickerDidCancel(picker as! ItemPickerCollectionViewController)
    }
}

class ItemPickerCollectionViewController: UICollectionViewController, ItemPickerUIViewController {
    // ItemPickerUIViewController
    weak var delegate: ItemPickerDelegate? = nil
    var dataSource: ItemPickerDataSource? = nil
    
    // MARK: - Objective-C support
    private var delegateConverter: ItemPickerDelegateConverter? = nil
    @objc var objcDelegate: ItemPickerObjcDelegate? {
        set(value) {
            if let value = value {
                delegateConverter = delegateConverter(value)
            } else {
                delegateConverter = nil
            }
            self.delegate = delegateConverter
        }
        get {
            return self.delegateConverter?.objcDelegate ?? nil
        }
    }
    
    func delegateConverter(_ objcDelegate: ItemPickerObjcDelegate) -> ItemPickerDelegateConverter {
        return ItemPickerDelegateConverter(objcDelegate)
    }
    
    // MARK: -
    var itemView: ItemPickerItemView = DefaultItemViewCollectionView()
    var savedBarButtonsStatus: [BarButtonItemStatus] = []
    
    var isLoading: Bool = false {
        willSet (newValue) {
            setLoading(status: newValue)
        }
    }
    // Private
    private var reuseId: String = "itemCell"
    private var sectionHeaderReuseId: String = "sectionHeader"
    private lazy var extraActionEnabler: ExtraActionEnabler = {
        ExtraActionEnabler(viewController: self)
    }()

    public var extraActions: [TitledAction] {
        get {
            return self.extraActionEnabler.extraActions
        }
        set(actions) {
            self.extraActionEnabler.extraActions = actions
        }
    }
    
    // API
    /// Override only reuseIdentifier for item cell. To provide cell class as well, set imageView property
    public func override(reuseId: String) {
        self.itemView = (self.itemView as! DefaultItemViewCollectionView).changing(reuseId: reuseId)
    }
    
    
    // UIViewController
    override func viewDidLoad() {
        super.viewDidLoad()
        
        var cellClass: AnyClass? = UICollectionViewCell.self
        if let tuple = self.itemView.reusableCellClassInfo {
            cellClass = tuple.cls
            reuseId = tuple.reuseId
        }
        
        if let cellClass = cellClass {
            /// Discard the associated cell class in the storyboard with reuseId if any
            collectionView!.register(cellClass, forCellWithReuseIdentifier: reuseId)
        }
        
        if let dataSource = self.dataSource {
            isLoading = true
            dataSource.loadItems {
                DispatchQueue.main.async {
                    self.collectionView!.reloadData()
                    self.isLoading = false
                    self.extraActionEnabler.setupNavigationItem(self.navigationItem)

                    self.customizeNavigationItem()
                }
            }
        }
        
    }
    
    // MARK: -
    var isSectionHeadersEnabled: Bool = true {
        willSet(enabled) {
            if let flowLayout = collectionView?.collectionViewLayout as? UICollectionViewFlowLayout {
                if !enabled {
                    flowLayout.headerReferenceSize = CGSize(width:0, height:0)
                } else {
                    flowLayout.headerReferenceSize = CGSize(width:32, height:32)
                }
            }
        }
    }
    
    /// re-render only if the cell is currently visible
    func rerender(at indexPath: IndexPath, selected: Bool? = nil) {
        if let dataSource = dataSource {
            if let cell = collectionView!.cellForItem(at: indexPath) {
                if let selected = selected {
                    cell.isSelected = selected
                }
                itemView.render(dataSource.item(at:indexPath), with: cell)
            }
        }
    }
    
    /// Override this and customize navigation item if necessary, but only in this function
    func customizeNavigationItem() {
        
    }
    
    // MARK: -
    override func numberOfSections(in collectionView: UICollectionView) -> Int {
        let titleCount = self.dataSource?.sectionTitles.count ?? 0
        isSectionHeadersEnabled = (titleCount > 0)
        return self.dataSource?.sectionCount ?? 0
    }
    
    override func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return self.dataSource?.itemCount(in: section) ?? 0
    }
    
    override func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: reuseId, for: indexPath)
        let item = self.dataSource?.item(at: indexPath) ?? DefaultItemPickerData()
        self.itemView.render(item, with: cell)
        
        return cell
    }
    
    override func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        rerender(at: indexPath)
        
        // trigger delegate after selection update
        DispatchQueue.global().async {
            DispatchQueue.main.async {
                self.delegate?.itemPicker(self, didSelect: indexPath)
                self.rerender(at: indexPath, selected: false)
            }
        }
    }
    
    override func collectionView(_ collectionView: UICollectionView, didDeselectItemAt indexPath: IndexPath) {
        rerender(at: indexPath)
    }
    
    override func collectionView(_ collectionView: UICollectionView, viewForSupplementaryElementOfKind kind: String, at indexPath: IndexPath) -> UICollectionReusableView {
        let view = collectionView.dequeueReusableSupplementaryView(ofKind: kind, withReuseIdentifier: sectionHeaderReuseId, for: indexPath)
        if let label = view.firstSubview(ofClass: UILabel.self) {
            label.text = self.dataSource?.sectionTitles[indexPath.section]
        }
        return view
    }
    
    // MARK: - Segue
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let delegate = delegate as? ItemPickerSegueDelegate {
            delegate.itemPicker(self, prepareFor: segue, sender: sender)
        }
    }
    
}
