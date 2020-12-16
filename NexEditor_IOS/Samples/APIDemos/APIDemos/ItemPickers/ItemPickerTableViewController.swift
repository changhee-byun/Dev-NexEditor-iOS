/**
 * File Name   : ItemPickerTableViewController.swift
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
import NexEditorFramework

struct DefaultItemPickerData: ItemPickerItem {
    var title: String = ""
    var desc: String? = ""
    func loadImage(size: CGSize, loaded: @escaping (UIImage?)->Void) { }
}

// MAKR: - ItemPickerItemViewTableViewCell

extension UITableViewCell: ItemPickerViewCell { }

protocol ItemPickerItemViewTableViewCell: ItemPickerItemView {
    
}

extension ItemPickerItemViewTableViewCell {
    func defaultRender(_ item: ItemPickerItem, with cell: UITableViewCell) {
        cell.textLabel?.text = item.title
        cell.detailTextLabel?.text = item.desc
        if cell.imageView?.image == nil {
            cell.imageView?.image = #imageLiteral(resourceName: "default_black.png")
        }
        item.loadImage(size: imageSize) { image in
            cell.imageView?.image = image
        }
    }
}

struct DefaultItemView: ItemPickerItemViewTableViewCell {
    
    static let reuseId = "defaultItemPickerCell"
    
    var imageSize: CGSize = CGSize(width: 120, height: 80)
    var reusableCellClassInfo: ReusableCellInfo? = ReusableCellInfo(reuseId: DefaultItemView.reuseId, cls: nil)
    
    func render(_ item: ItemPickerItem, with cell: ItemPickerViewCell) {
        if let cell = cell as? UITableViewCell {
            defaultRender(item, with: cell)
        }
    }
    
    /// create a copy with the specified reuseId
    func changing(reuseId: String) -> ItemPickerItemViewTableViewCell {
        let cellInfo = ReusableCellInfo(reuseId: reuseId, cls: self.reusableCellClassInfo!.cls)
        return DefaultItemView(imageSize: self.imageSize, reusableCellClassInfo: cellInfo)
    }
}

class ItemPickerTableViewController: UITableViewController, ItemPickerUIViewController {
    
    weak var delegate: ItemPickerDelegate? = nil
    var dataSource: ItemPickerDataSource? = nil
    var itemView: ItemPickerItemView = DefaultItemView()
    
    private var reuseId: String = "defaultItemPickerCell"
    
    var savedBarButtonsStatus: [BarButtonItemStatus] = []
    
    var isLoading: Bool = false {
        willSet (newValue) {
            setLoading(status: newValue)
        }
    }
    
    /// Override only reuseIdentifier for item cell. To provide cell class as well, set imageView property
    public func override(reuseId: String) {
        self.itemView = (self.itemView as! DefaultItemView).changing(reuseId: reuseId)
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        var cellClass: AnyClass? = UITableViewCell.self
        if let tuple = self.itemView.reusableCellClassInfo {
            cellClass = tuple.cls
            reuseId = tuple.reuseId
        }
        
        if let cellClass = cellClass {
            /// Discard the associated cell class in the storyboard with reuseId if any
            tableView.register(cellClass, forCellReuseIdentifier: reuseId)
        }
        
        if let dataSource = self.dataSource {
            isLoading = true
            dataSource.loadItems {
                DispatchQueue.main.async {
                    self.tableView.reloadData()
                    self.isLoading = false
                }
            }
        }
    }
    
    override func didMove(toParent parent: UIViewController?) {
        if parent == nil {
            self.delegate?.itemPickerDidCancel(self)
        }
    }
    
    // MARK: - API
    
    func rerender(at indexPath: IndexPath) {
        if let dataSource = dataSource {
            itemView.render(dataSource.item(at:indexPath), with: tableView.cellForRow(at: indexPath)!)
        }
    }
    
    // MARK: - Data Source
    override func numberOfSections(in tableView: UITableView) -> Int {
        return self.dataSource?.sectionCount ?? 0
    }
    
    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        let titles = self.dataSource?.sectionTitles ?? []
        return titles.count > 0 ? titles[section] : nil
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return self.dataSource?.itemCount(in: section) ?? 0
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: reuseId, for: indexPath)
        let item = self.dataSource?.item(at: indexPath) ?? DefaultItemPickerData()
        self.itemView.render(item, with: cell)
        
        return cell
    }
    
    // MARK: - UITableViewDelegate
    
    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        self.delegate?.itemPicker(self, didSelect: indexPath)
    }
    
    // MARK: - Segue
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let delegate = delegate as? ItemPickerSegueDelegate {
            delegate.itemPicker(self, prepareFor: segue, sender: sender)
        }
    }
}
