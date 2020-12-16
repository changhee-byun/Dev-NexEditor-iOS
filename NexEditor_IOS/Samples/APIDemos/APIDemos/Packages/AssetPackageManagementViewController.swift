/*
 * File Name   : AssetPackageManagementViewController.swift
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

class AssetPackageManagementViewController: UITableViewController {

    let packageManager = AssetPackageManager.manager
    var packageURLs: [AssetPackageManager.Location: [URL]] = [:]
    
    override func viewDidLoad() {
        super.viewDidLoad()
        packageURLs = packageManager.scanPackageURLs()
        
        let actions = UIBarButtonItem(barButtonSystemItem: .action, target: self, action: #selector(didTapAction(_:)))
        self.navigationItem.rightBarButtonItem = actions
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    @objc func didTapAction(_ sender: UIBarButtonItem) {
        self.alertSimpleActionSheet("Actions",
                                    message: "",
                                    actionTitles: ["Reinstall All Packages"],
                                    dismissTitle: "Cancel") { [weak self] (index) in
                                        if index == 0 {
                                            // Reinstall All Packages
                                            self?.reinstallAllPackages()
                                        }
        }
    }
    
    // MARK: -
    
    func packageURL(at indexPath: IndexPath) -> URL? {
        if let location = AssetPackageManager.Location(rawValue: indexPath.section),
            let urls = packageURLs[location] {
            return urls[indexPath.row]
        }
        return nil
    }
    
    func installPackage(atURL url: URL) {
        packageManager.installPackage(atURL: url)
    }
    
    func uninstallPackage(atURL url: URL) {
        packageManager.uninstallPackage(atURL: url)
    }
    
    func reinstallAllPackages() {
        self.view.isUserInteractionEnabled = false
        runWithBarButtonItemActivityIndicator {
            NXEAssetLibrary.removeAllAssetPackageDirectoryURLs();
            AssetPackageManager.manager.installAllPackages()
            self.tableView.reloadData()
            self.view.isUserInteractionEnabled = true
        }
    }
    
    // MARK: - Table view data source

    override func numberOfSections(in tableView: UITableView) -> Int {
        return AssetPackageManager.Location.count
    }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        // #warning Incomplete implementation, return the number of rows
        guard let location = AssetPackageManager.Location(rawValue: section) else { return 0 }
        return packageURLs[location]?.count ?? 0
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "package", for: indexPath)
        
        if let url = packageURL(at: indexPath) {
            cell.textLabel?.text = url.lastPathComponent.capitalized.replacingOccurrences(of: "_", with: " ")
            cell.detailTextLabel?.text = packageManager.isInstallled(packageAtURL: url) ? "Installed" : "Not installed"
        }

        return cell
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        if let url = packageURL(at: indexPath) {
            let message: String
            let actionTitle: String
            let install = packageManager.isInstallled(packageAtURL: url) ? false : true
            if install {
                message = "Install the package to Asset Library?"
                actionTitle = "Install"
            } else {
                message = "Uninstall the package from Asset Library?"
                actionTitle = "Uninstall"
            }
            self.alertSimpleActionSheet("Confirm",
                                        message: message,
                                        actionTitles: [actionTitle],
                                        dismissTitle: "Cancel",
                                        complete: { [weak self] (index) in
                                            if install {
                                                self?.installPackage(atURL: url)
                                            } else {
                                                self?.uninstallPackage(atURL: url)
                                            }
                                            tableView.reloadRows(at: [indexPath], with: .automatic)
                                            
            })
        }
    }

}
