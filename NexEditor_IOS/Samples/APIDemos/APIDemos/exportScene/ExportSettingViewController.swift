/**
 * File Name   : ExportSettingViewController.swift
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

protocol ExportSettingItem {
    var title: String { get }
    var setting: ExportSetting { get }
}

protocol ExportSettingDataSource {
    func loadItems(_ loaded: @escaping ()->Void)
    func itemCount(in section: Int) -> Int
    func item(at indexPath:IndexPath) -> ExportSettingItem
    func updateItem(at indexPath: IndexPath, with item:ExportSetting, completion: @escaping () -> Void )
    func reset( completion: @escaping () -> Void )
}

class ExportSettingCell : UITableViewCell {
    @IBOutlet var title: UILabel!
}

class ExportSettingViewController : UITableViewController, ExportSettingDelegate {
    
    var dataSource:ExportSettingDataSource? = nil
    var currentIndexPath:IndexPath? = nil
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.navigationItem.rightBarButtonItem =
            UIBarButtonItem.init(title: "Reset", style: .plain, target: self, action:  #selector(didTapResetButton(sender:)))
        
        if let dataSource = self.dataSource {
            dataSource.loadItems {
                DispatchQueue.main.async {
                    self.tableView.reloadData()
                }
            }
        }
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return self.dataSource?.itemCount(in: section) ?? 0
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier:"exportSettingCell", for: indexPath) as! ExportSettingCell

        cell.title.text = "Unknown"
        if let item = self.dataSource?.item(at: indexPath) {
            cell.title.text = item.title
        }
        return cell
    }
    
    @objc func didTapResetButton(sender:UIBarButtonItem) {
        if let dataSource = self.dataSource {
            dataSource.reset {
                DispatchQueue.main.async {
                    self.tableView.reloadData()
                }
            }
        }
    }
    
    func exportSettingValueChanged(_ picker:ExportSettingDetailViewController, value:ExportSetting) {
        if let dataSource = self.dataSource {
            dataSource.updateItem(at: self.currentIndexPath!, with:value, completion: {
                DispatchQueue.main.async {
                    self.tableView.reloadData()
                }
            })
        }
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "exportDetailSegue" {
            self.currentIndexPath = self.tableView.indexPath(for: sender as! ExportSettingCell)
            let destination = segue.destination as! ExportSettingDetailViewController
            destination.setting = self.dataSource?.item(at:self.currentIndexPath!).setting
            destination.delegate = self
        }
    }
}
